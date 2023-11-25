#pragma once

#include "core/registers.h"
#include "script/command.h"
#include "text/cursor.h"
#include "text/cursorrange.h"
#include "text/fstringview.h"
#include "views/editorcursor.h"
#include <functional>
#include <meditfwd.h>
#include <memory>
#include <optional>
#include <stdexcept>

enum class VimMode {
    Normal,
    Insert,
    Visual,
};

namespace vim {

std::shared_ptr<IMode> createMode(VimMode resultMode);

constexpr char matching(char c) {
    switch (c) {
    case '"':
        return '"';
    case '(':
        return ')';
    case '{':
        return '}';
    case '[':
        return ']';
    case '<':
        return '>';
    }

    return c;
}

enum MatchType {
    PartialMatch,
    Match,
    NoMatch,
};

} // namespace vim

enum class VimCommandType {
    Change,
    Visual,
    Delete,
    Yank,
    Other,
    Motion,
};

VimCommandType getType(VimMode modeName, FStringView &buffer);

struct VimMotionResult {
    vim::MatchType match;
    std::function<EditorCursor(EditorCursor, int)> f;

    operator bool const() {
        return match == vim::MatchType::Match && f;
    }
};

/// Param is used because some motions are different for visual mode and delete
/// Set command = 0 for motion only
VimMotionResult getMotion(FStringView str, VimCommandType);

using SelectionFunctionReturnT = std::pair<CursorRange, Cursor>;

struct SelectionFunctionT {
    vim::MatchType match;
    std::function<SelectionFunctionReturnT(EditorCursor, int)> f;
};

SelectionFunctionT getSelectionFunction(FStringView buffer, VimCommandType);

std::pair<CursorRange, Cursor> getSelection(FStringView buffer,
                                            Cursor,
                                            VimMode,
                                            int);

VimMode applyAction(VimCommandType type,
                    CursorRange range,
                    Registers &registers);

struct ActionResultT {
    vim::MatchType match = vim::NoMatch;
    CommandT f;
};

ActionResultT findVimAction(FStringView, VimMode modeName);
