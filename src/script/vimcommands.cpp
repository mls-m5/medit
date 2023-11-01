#include "script/vimcommands.h"
#include "meditfwd.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "modes/visualmode.h"
#include "text/cursor.h"
#include "text/cursorops.h"
#include "text/cursorrange.h"
#include "text/cursorrangeops.h"
#include "text/fstring.h"
#include "text/fstringview.h"
#include "text/utf8char.h"
#include "text/utf8charops.h"
#include "views/editor.h"
#include <array>
#include <optional>
#include <stdexcept>
#include <string>

namespace {

// Wrapper functions for handling functions with default arguments
template <typename F, typename... Args>
auto wrap(F f, Args... args) {
    return [=](Cursor cursor) { return f(cursor, args...); };
}

template <typename... Args>
std::function<Cursor(Cursor)> combine(Args... args) {
    return [=](Cursor cursor) -> Cursor {
        auto functions =
            std::array<std::function<Cursor(Cursor)>, sizeof...(args)>{
                wrap(args)...};
        for (auto f : functions) {
            cursor = f(cursor);
        }
        return cursor;
    };
}

Cursor lastNonSpaceOnLine(Cursor cursor) {
    for (auto cur = cursor; cur != end(cursor); ++cur) {
        auto c = content(cur);
        if (!isSpace(c)) {
            cursor = cur;
        }
    }

    return cursor;
}

Cursor firstNonSpaceOnLine(Cursor cursor) {
    for (auto cur = home(cursor); cur != end(cursor); ++cur) {
        auto c = content(cur);
        cursor = cur;
        if (!isSpace(c)) {
            return cursor;
        }
    }

    return cursor;
}

/// std::less is used to be able to compare with FString
const static auto map =
    std::map<FString, std::function<Cursor(Cursor)>, std::less<>>{
        {"h", wrap(left, false)},
        {"l", wrap(right, false)},
        {"j", down},
        {"k", up},
        {"b", combine(wrap(left, true), wordBegin)},
        {"w", combine(wordEnd, wrap(right, true), wordEnd, wordBegin)},
        {"e", combine(wrap(right, true), wordEnd)},
        {"b", wordBegin},
        {"0", home},
        {"$", end},
        {"^", firstNonSpaceOnLine},
        {"g_", lastNonSpaceOnLine},
        {"gg", [](Cursor c) { return c.buffer().begin(); }},
        {"G", [](Cursor c) { return c.buffer().end(); }},
    };

CursorRange inner(char c, Cursor cursor) {
    const auto matchingChar = ::vim::matching(c);
    // TODO: Handle special case for ""

    // TODO: Handle when there is numbers in the mode

    return ::inner(cursor, c, matchingChar);
}

CursorRange around(char c, Cursor cursor) {
    auto range = inner(c, cursor);

    range.beginPosition(left(range.begin()));
    range.endPosition(right(range.end()));

    return range;
}

} // namespace

// @param buffer. Note that this parameter changes the value sent in by erasing
// the information that is aquired through this function
VimCommandType getType(VimMode modeName, FString &buffer) {
    auto commandType = VimCommandType::Visual;

    if (modeName == VimMode::Normal) {
        auto first = static_cast<uint32_t>(buffer.front().c);

        switch (first) {
        case 'c':
            commandType = VimCommandType::Change;
            break;
        case 'd':
            commandType = VimCommandType::Delete;
            break;
        case 'y':
            commandType = VimCommandType::Yank;
            break;
        default:
            commandType = VimCommandType::Other;
        }

        if (commandType != VimCommandType::Other) {
            buffer.erase(0, 1);
        }
    }

    return commandType;
}

std::optional<std::function<CursorRange(Cursor, VimMode, int)>> getSelection(
    const FString &buffer) {

    if (buffer.empty()) {
        throw "error";
    }

    //    auto front = static_cast<uint32_t>(buffer.front().c);

    auto f = getMotion(buffer);

    if (f) {
        return [f = f.f](Cursor cursor, VimMode, int num) -> CursorRange {
            return CursorRange{cursor, f(cursor, num)};
        };
    }

    return std::nullopt;
}

VimMotionResult getMotion(FStringView buffer) {
    /// Take a function that fires once and add argument for how many
    /// repetitions should be used
    auto pack = [](std::function<Cursor(Cursor)> single) -> VimMotionResult {
        auto f = [single](Cursor cur, int num) {
            for (int i = 0; i < num; ++i) {
                cur = single(cur);
            }
            return cur;
        };

        return {.match = vim::Match, .f = f};
    };

    if (buffer.empty()) {
        return {.match = vim::NoMatch};
    }

    if (buffer.front() == 'f') {
        if (buffer.size() == 1) {
            return {.match = vim::MatchType::PartialMatch};
        }

        auto searchTerm = buffer.at(1);
        return pack([searchTerm](Cursor cursor) -> Cursor {
            auto f = find(cursor, searchTerm.c, false);
            return f ? *f : cursor;
        });
    }

    if (buffer.front() == 'F') {
        if (buffer.size() == 1) {
            return {.match = vim::MatchType::PartialMatch};
        }

        auto searchTerm = buffer.at(1);
        return pack([searchTerm](Cursor cursor) -> Cursor {
            auto f = rfind(cursor, searchTerm.c, false);
            return f ? *f : cursor;
        });
    }

    if (auto single = map.find(buffer); single != map.end()) {
        auto f = [single = single->second](Cursor cur, int num) {
            for (int i = 0; i < num; ++i) {
                cur = single(cur);
            }
            return cur;
        };

        return {
            .match = vim::Match,
            .f = f,
        };
    }

    for (auto &it : map) {
        if (FString{it.first}.substr(0, buffer.size()) == buffer) {
            return {.match = vim::PartialMatch};
        }
    }

    return {.match = vim::NoMatch};
}

// std::optional<std::function<Cursor(Cursor, int)>> getInnerFunction(
//     FString buffer) {
//     const static auto map = std::map<FString, std::function<Cursor(Cursor)>>{
//         {"w", ::wordEnd},
//         {"(", ::wordBegin},
//     };

//    if (auto single = map.find(buffer); single != map.end()) {
//        return [single = single->second](Cursor cur, int num) {
//            for (int i = 0; i < num; ++i) {
//                cur = single(cur);
//            }
//            return cur;
//        };
//    }

//    return std::nullopt;
//}

VimMode applyAction(VimCommandType type,
                    CursorRange range,
                    Registers &registers) {
    using T = VimCommandType;
    switch (type) {
    case T::Change:
        registers.save(standardRegister, content(range));
        erase(extendRight(range));
        return VimMode::Insert;
    case T::Yank:
        registers.save(standardRegister, content(range));
        return VimMode::Normal;
    case T::Delete:
        registers.save(standardRegister, content(range));
        erase(extendRight(range));
        return VimMode::Normal;
    default:
        throw std::runtime_error{"invalid action command type"};
    }

    throw std::runtime_error{"hello"};
    return VimMode::Normal;
}

std::shared_ptr<IMode> vim::createMode(VimMode resultMode) {
    switch (resultMode) {
    case VimMode::Normal:
        return createNormalMode();
    case VimMode::Insert:
        return createInsertMode();
    case VimMode::Visual:
        return createVisualMode();
    }
    throw std::runtime_error{"cannot create mode: This should never happend"};
}

vim::MatchType doVimAction(std::shared_ptr<IEnvironment> env,
                           VimMode modeName) {
    auto &editor = env->editor();
    auto &mode = editor.mode();
    auto cursor = editor.cursor();
    auto buffer = mode.buffer();

    auto commandType = getType(modeName, buffer);
    auto repetitions = mode.repetitions();
    auto [selection, newCursor] =
        getSelection(buffer, cursor, modeName, std::max(1, repetitions));

    applyAction(commandType, selection, env->registers());

    return vim::MatchType::Match; // Handle partial matches also
}

std::function<void(std::shared_ptr<IEnvironment>)> createVimAction(
    VimMode modeName) {
    return [modeName](std::shared_ptr<IEnvironment> env) {
        doVimAction(env, modeName);
    };
}

std::pair<CursorRange, Cursor> getInnerSelection(Cursor cursor,
                                                 Utf8Char type,
                                                 int repetitions) {
    auto intType = static_cast<char>(static_cast<uint32_t>(type));
    switch (intType) {
    case 'w': {
        auto begin = wordBegin(cursor);
        return {CursorRange{begin, wordEnd(cursor)}, begin};
    }
    case '(':
    case 'b': {
        auto range = inner('(', cursor);
        return {range, range.end()};
    }
    }

    return {cursor, cursor};
}

std::pair<CursorRange, Cursor> getAroundSelection(Cursor cursor,
                                                  Utf8Char type,
                                                  int repetitions) {

    // TODO: Handle special cases for tags

    auto selection = getInnerSelection(cursor, type, repetitions);

    if (selection.first.empty()) {
        return selection;
    }

    selection.first = extend(selection.first, 1, false);

    return selection;
}

std::pair<CursorRange, Cursor> getSelection(FStringView buffer,
                                            Cursor cursor,
                                            VimMode modeName,
                                            int repetitions) {

    if (buffer.empty()) {
        return {cursor, cursor};
    }

    auto motion = getMotion(buffer);

    if (motion) {
        // TODO: Handle when motion is backwards
        return {CursorRange{cursor, motion.f(cursor, repetitions)}, cursor};
    }

    if (buffer.front() == 'i') {
        return getInnerSelection(cursor, buffer.at(1).c, repetitions);
    }

    if (buffer.front() == 'a') {
        return getAroundSelection(cursor, buffer.at(1).c, repetitions);
    }

    throw std::runtime_error{"hsaothesut"};
}
