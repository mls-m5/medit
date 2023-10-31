#include "script/vimcommands.h"
#include "text/cursorops.h"
#include "text/cursorrange.h"
#include "text/fstring.h"
#include "text/utf8char.h"
#include <optional>
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

std::optional<std::function<Cursor(Cursor, int)>> getMotion(FString buffer) {
    static auto map = std::map<FString, std::function<Cursor(Cursor)>>{
        {"w", ::wordEnd},
        {"b", ::wordBegin},
    };

    auto single = map.at(buffer);

    return [single](Cursor cur, int num) {
        for (int i = 0; i < num; ++i) {
            cur = single(cur);
        }
        return cur;
    };
}

CursorRange vim::inner(char c, Cursor cursor) {
    const auto matchingChar = ::vim::matching(c);
    // TODO: Handle special case for ""

    // TODO: Handle when there is numbers in the mode

    return ::inner(cursor, c, matchingChar);
}

CursorRange vim::around(char c, Cursor cursor) {
    auto range = inner(c, cursor);

    range.beginPosition(left(range.begin()));
    range.endPosition(right(range.end()));

    return range;
}
