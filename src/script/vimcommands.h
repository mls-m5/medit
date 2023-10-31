#pragma once

#include "core/registers.h"
#include "ienvironment.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "modes/visualmode.h"
#include "text/cursor.h"
#include "text/cursorops.h"
#include "text/cursorrange.h"
#include "text/cursorrangeops.h"
#include "text/fstring.h"
#include "text/utf8char.h"
#include "views/editor.h"
#include <functional>
#include <meditfwd.h>
#include <memory>
#include <optional>

enum class VimMode {
    Normal,
    Insert,
    Visual,
};

namespace vim {

using SelectionF = CursorRange (*)(Cursor);

template <VimMode mode, SelectionF f>
void select(std::shared_ptr<IEnvironment> env) {}

template <VimMode resultMode>
std::shared_ptr<IMode> createMode() {
    switch (resultMode) {
    case VimMode::Normal:
        return createNormalMode();
    case VimMode::Insert:
        return createInsertMode();
    case VimMode::Visual:
        return createVisualMode();
    }
}

/// Example:
/// yy -> resultMode = normal, f = selectLine,
///       shouldYank = true, shouldErase = false
/// dd -> resultMode = normal, f = selectLine,
///       shouldYank = true, shouldErase = false
/// etc...
template <VimMode resultMode, SelectionF f, bool shouldYank, bool shouldErase>
void change(std::shared_ptr<IEnvironment> env) {
    auto &e = env->editor();
    auto range = e.selection();
    auto &mode = e.mode();

    if (range.empty()) {
        range = f(range.begin());
    }

    if (shouldYank) {
        /// TODO: Extract the register name from the current mode
        env->registers().save(standardRegister, content(range));
    }

    auto cursor = [&] {
        if constexpr (shouldErase) {
            return erase(range);
        }
        else {
            return e.cursor();
        }
    }();
    e.clearSelection();
    e.cursor(cursor);
    e.mode(createMode<resultMode>());
}

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

CursorRange inner(char c, Cursor cursor);

CursorRange around(char c, Cursor cursor);

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

std::optional<std::function<CursorRange(Cursor, VimMode, int)>> getSelection(
    const FString &buffer);

template <VimMode modeName>
std::optional<std::function<void(std::shared_ptr<IEnvironment>)>>
standardVimCommand(std::shared_ptr<IEnvironment> env) {
    auto &editor = env->editor();
    auto &mode = editor.mode();

    auto buffer = mode.buffer();

    auto commandType = getType(modeName, buffer);

    auto repetitions = mode.repetitions();
    auto movement = buffer;

    return std::nullopt;
}
