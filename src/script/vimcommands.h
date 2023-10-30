#pragma once

#include "core/registers.h"
#include "ienvironment.h"
#include "text/cursor.h"
#include "text/cursorrange.h"
#include "text/fstring.h"
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

} // namespace vim

enum class VimCommandType {
    Change,
    Visual,
    Delete,
    Yank,
    Other,
};

VimCommandType getType(VimMode modeName, FString &buffer);

std::optional<std::function<Cursor(Cursor, int)>> getMotion(FString);

CursorRange getSelection(const FString &buffer, Cursor, VimMode, int);

VimMode applyAction(VimCommandType type,
                    CursorRange range,
                    Registers &registers);

// template <VimMode modeName>
void doVimAction(std::shared_ptr<IEnvironment> env, VimMode modeName);

std::function<void(std::shared_ptr<IEnvironment>)> createVimAction(
    VimMode modeName);