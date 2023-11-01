#pragma once

#include "core/registers.h"
#include "ienvironment.h"
#include "text/cursor.h"
#include "text/cursorrange.h"
#include "text/fstring.h"
#include "text/fstringview.h"
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

    return 0;
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
};

// vim::MatchType matchVimMotion(FStringView str);

VimCommandType getType(VimMode modeName, FString &buffer);

struct VimMotionResult {
    vim::MatchType match;
    std::function<Cursor(Cursor, int)> f;

    operator bool const() {
        return match == vim::MatchType::Match && f;
    }
};

VimMotionResult getMotion(FStringView);

std::pair<CursorRange, Cursor> getSelection(FStringView buffer,
                                            Cursor,
                                            VimMode,
                                            int);

VimMode applyAction(VimCommandType type,
                    CursorRange range,
                    Registers &registers);

vim::MatchType doVimAction(std::shared_ptr<IEnvironment> env, VimMode modeName);

/// Probably deprecated
std::function<void(std::shared_ptr<IEnvironment>)> createVimAction(
    VimMode modeName);
