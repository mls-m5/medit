#include "script/vimcommands.h"
#include "meditfwd.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "modes/visualmode.h"
#include "script/ienvironment.h"
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
#include <memory>
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
        {"w", wrap(nextWord, true)},
        {"e", combine(wrap(right, true), wordEnd)},
        {"b", wordBegin},
        {"0", home},
        {"$", end},
        {"^", firstNonSpaceOnLine},
        {"g_", lastNonSpaceOnLine},
        {"gg", [](Cursor c) { return c.buffer().begin(); }},
        {"G", [](Cursor c) { return c.buffer().end(); }},
        {"%", findMatching},
    };

CursorRange vimInner(char c, Cursor cursor) {
    const auto matchingChar = ::vim::matching(c);

    return ::inner(cursor, c, matchingChar);
}

CursorRange around(char c, Cursor cursor) {
    auto range = vimInner(c, cursor);

    range.beginPosition(left(range.begin()));
    range.endPosition(right(range.end()));

    return range;
}

} // namespace

// @param buffer. Note that this parameter changes the value sent in by erasing
// the information that is aquired through this function
VimCommandType getType(FStringView &buffer) {
    auto commandType = VimCommandType::Visual;

    //    if (modeName == VimMode::Normal) {
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
        buffer = buffer.substr(1);
    }
    //    }

    return commandType;
}

std::optional<std::function<CursorRange(Cursor, VimMode, int)>> getSelection(
    const FString &buffer, VimCommandType type) {

    if (buffer.empty()) {
        throw "error";
    }

    auto f = getMotion(buffer, type);

    if (f) {
        return [f = f.f](Cursor cursor, VimMode, int num) -> CursorRange {
            return CursorRange{cursor, f(cursor, num)};
        };
    }

    return std::nullopt;
}

VimMotionResult getMotion(FStringView buffer, VimCommandType type) {
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

    auto front = buffer.front();

    if (front == 'f' || front == 't') {
        if (buffer.size() == 1) {
            return {.match = vim::MatchType::PartialMatch};
        }

        auto searchTerm = buffer.at(1);
        return pack([searchTerm, type, front](Cursor cursor) -> Cursor {
            auto f = find(cursor, searchTerm.c, false, front == 't');
            if (!f) {
                return cursor;
            }

            auto newPosition = *f;
            if (type == VimCommandType::Delete ||
                type == VimCommandType::Change) {
                newPosition = right(newPosition);
            }
            return newPosition;
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

VimMode applyAction(VimCommandType type,
                    CursorRange range,
                    Registers &registers) {
    using T = VimCommandType;
    switch (type) {
    case T::Change:
        registers.save(standardRegister, content(range));
        erase(range);
        return VimMode::Insert;
    case T::Yank:
        registers.save(standardRegister, content(range));
        return VimMode::Normal;
    case T::Delete:
        registers.save(standardRegister, content(range));
        erase(range);
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

ActionResultT findVimAction(FStringView buffer, VimMode modeName) {
    auto bestResult = vim::MatchType::NoMatch;

    if (buffer.empty()) {
        return {vim::NoMatch};
    }

    if (buffer.front() == 'r') {
        if (buffer.size() == 1) {
            return {vim::MatchType::PartialMatch};
        }
        auto replacement = buffer.at(1);

        auto f = [replacement](std::shared_ptr<IEnvironment> env) {
            auto &editor = env->editor();
            auto cursor = editor.cursor();
            auto repetitions = editor.mode().repetitions();
            auto end = cursor;
            for (size_t i = 0; i < repetitions; ++i) {
                end = right(end);
            }
            replace({cursor, end},
                    FString{static_cast<size_t>(repetitions), replacement});
            editor.cursor(left(end));
        };

        return {vim::Match, f};
    }

    {
        auto motion = getMotion(buffer, VimCommandType::Motion);
        if (motion.match == vim::Match) {
            auto f = [f = motion.f](std::shared_ptr<IEnvironment> env) {
                auto &editor = env->editor();
                auto cursor = editor.virtualCursor();
                auto repetitions = editor.mode().repetitions();
                cursor = f(cursor, repetitions);
                editor.cursor(cursor);
            };

            return {vim::Match, f};
        }
        else if (motion.match == vim::PartialMatch) {
            bestResult = vim::PartialMatch;
        }
    }

    /// Note that this removes the firts part of `buffer`
    auto commandType = getType(buffer);

    if (commandType == VimCommandType::Other) {
        return {bestResult};
    }

    auto selectionFunction = getSelectionFunction(buffer, commandType);

    if (selectionFunction.match == vim::NoMatch) {
        return {bestResult};
    }
    if (selectionFunction.match == vim::PartialMatch) {
        return {vim::PartialMatch};
    }

    auto f = [modeName,
              commandType,
              buffer = FString{buffer},
              selectionF =
                  selectionFunction.f](std::shared_ptr<IEnvironment> env) {
        auto &editor = env->editor();
        auto &mode = editor.mode();
        auto cursor = editor.cursor();

        auto repetitions = mode.repetitions();
        auto [selection, newCursor] =
            selectionF(cursor, std::max(1, repetitions));

        auto newMode = applyAction(commandType, selection, env->registers());

        editor.cursor(newCursor);

        editor.mode(vim::createMode(newMode));
    };

    return {vim::MatchType::Match, f};
}

std::pair<CursorRange, Cursor> getInnerSelection(Cursor cursor,
                                                 Utf8Char type,
                                                 int repetitions) {
    auto intType = static_cast<char>(static_cast<uint32_t>(type));
    switch (intType) {
    case 'w': {
        auto begin = wordBegin(cursor);
        return {CursorRange{begin, right(wordEnd(cursor), false)}, begin};
    }
    case '(':
    case 'b': {
        auto range = vimInner('(', cursor);
        return {range, range.begin()};
    }
    case '{':
    case 'B': {
        auto range = vimInner('{', cursor);
        return {range, range.begin()};
    }
    case '[': {
        auto range = vimInner('[', cursor);
        return {range, range.begin()};
    }
    case '"': {
        auto range = vimInner('"', cursor);
        return {range, range.begin()};
    }
    case '\'': {
        auto range = vimInner('\'', cursor);
        return {range, range.begin()};
    }
    case '`': {
        auto range = vimInner('`', cursor);
        return {range, range.begin()};
    }
    }

    return {cursor, cursor};
}

std::pair<CursorRange, Cursor> getAroundSelection(Cursor cursor,
                                                  Utf8Char type,
                                                  int repetitions) {

    // TODO: Handle special cases for tags
    // TODO: For words, only select spaces, and not special characters

    auto selection = getInnerSelection(cursor, type, repetitions);

    if (selection.first.empty()) {
        return selection;
    }

    selection.first = extend(selection.first, 1, false);

    return selection;
}

SelectionFunctionT getSelectionFunction(FStringView buffer,
                                        VimCommandType type) {
    if (buffer.empty()) {
        return {vim::PartialMatch};
    }

    auto motion = getMotion(buffer, type);

    if (motion.match == vim::PartialMatch) {
        return {.match = vim::PartialMatch};
    }
    else if (motion) {
        // TODO: Handle when motion is backwards
        auto f = [f = motion.f](Cursor cursor,
                                int repetitions) -> SelectionFunctionReturnT {
            return {CursorRange{cursor, f(cursor, repetitions)}, cursor};
        };

        return {
            .match = vim::Match,
            .f = f,
        };
    }

    if (buffer.front() == 'i') {
        if (buffer.size() == 1) {
            return {.match = vim::PartialMatch};
        }
        auto f = [type = buffer.at(1).c](
                     Cursor cursor,
                     int repetitions) -> SelectionFunctionReturnT {
            return getInnerSelection(cursor, type, repetitions);
        };
        return {
            .match = vim::Match,
            .f = f,
        };
    }

    if (buffer.front() == 'a') {
        if (buffer.size() == 1) {
            return {.match = vim::PartialMatch};
        }
        auto f = [type = buffer.at(1).c](
                     Cursor cursor,
                     int repetitions) -> SelectionFunctionReturnT {
            return getAroundSelection(cursor, type, repetitions);
        };
        return {
            .match = vim::Match,
            .f = f,
        };
    }

    return {.match = vim::NoMatch};
}
