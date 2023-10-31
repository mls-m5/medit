#include "script/vimcommands.h"
#include "text/cursorops.h"
#include "text/cursorrange.h"
#include "text/cursorrangeops.h"
#include "text/fstring.h"
#include "text/utf8char.h"
#include <optional>
#include <stdexcept>
#include <string>

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

    auto front = static_cast<uint32_t>(buffer.front().c);

    auto f = getMotion(buffer);

    if (f) {
        return [f = *f](Cursor cursor, VimMode, int num) -> CursorRange {
            return CursorRange{cursor, f(cursor, num)};
        };
    }

    return std::nullopt;
}

template <typename F, typename... Args>
auto wrap(F f, Args... args) {
    return [=](Cursor cursor) { return f(cursor, args...); };
}

std::optional<std::function<Cursor(Cursor, int)>> getMotion(FString buffer) {
    const static auto map = std::map<FString, std::function<Cursor(Cursor)>>{
        {"h", wrap(::left, false)},
        {"l", wrap(::right, false)},
        {"w", ::wordEnd},
        {"b", ::wordBegin},
    };

    if (auto single = map.find(buffer); single != map.end()) {
        return [single = single->second](Cursor cur, int num) {
            for (int i = 0; i < num; ++i) {
                cur = single(cur);
            }
            return cur;
        };
    }

    return std::nullopt;
}

std::optional<std::function<Cursor(Cursor, int)>> getInnerFunction(
    FString buffer) {
    const static auto map = std::map<FString, std::function<Cursor(Cursor)>>{
        {"w", ::wordEnd},
        {"(", ::wordBegin},
    };

    if (auto single = map.find(buffer); single != map.end()) {
        return [single = single->second](Cursor cur, int num) {
            for (int i = 0; i < num; ++i) {
                cur = single(cur);
            }
            return cur;
        };
    }

    return std::nullopt;
}

// CursorRange vim::inner(char c, Cursor cursor) {
//     const auto matchingChar = ::vim::matching(c);
//     // TODO: Handle special case for ""

//    // TODO: Handle when there is numbers in the mode

//    return ::inner(cursor, c, matchingChar);
//}

// CursorRange vim::around(char c, Cursor cursor) {
//     auto range = inner(c, cursor);

//    range.beginPosition(left(range.begin()));
//    range.endPosition(right(range.end()));

//    return range;
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

void doVimAction(std::shared_ptr<IEnvironment> env, VimMode modeName) {
    auto &editor = env->editor();
    auto &mode = editor.mode();
    auto cursor = editor.cursor();
    auto buffer = mode.buffer();

    auto commandType = getType(modeName, buffer);
    auto repetitions = mode.repetitions();
    auto selection =
        getSelection(buffer, cursor, modeName, std::max(1, repetitions));

    applyAction(commandType, selection, env->registers());
}

std::function<void(std::shared_ptr<IEnvironment>)> createVimAction(
    VimMode modeName) {
    return [modeName](std::shared_ptr<IEnvironment> env) {
        doVimAction(env, modeName);
    };
}

CursorRange getSelection(const FString &buffer,
                         Cursor cursor,
                         VimMode modeName,
                         int repetitions) {

    auto motion = getMotion(buffer);

    if (motion) {
        // TODO: Handle when motion is backwards
        return CursorRange{cursor, (*motion)(cursor, repetitions)};
    }

    throw std::runtime_error{"hsaothesut"};
}
