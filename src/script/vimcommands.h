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
#include "views/editor.h"
#include <meditfwd.h>
#include <memory>

enum VimMode {
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

template <char c>
constexpr char matching() {
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

template <char Char>
CursorRange inner(Cursor cursor) {
    static constexpr auto matchingChar = ::vim::matching<Char>();
    // TODO: Handle special case for ""

    // TODO: Handle when there is numbers in the mode

    return ::inner(cursor, Char, matchingChar);
}

template <char Char>
CursorRange around(Cursor cursor) {
    auto range = inner<Char>(cursor);

    range.beginPosition(left(range.begin()));
    range.endPosition(right(range.end()));

    return range;
}

} // namespace vim

void hello() {
    /// Testing instantiations
    auto selectionFunction = vim::select<VimMode::Normal, vim::inner<'"'>>;
    auto ciq = vim::change<VimMode::Insert, vim::inner<'"'>, true, true>;
    auto caq = vim::change<VimMode::Insert, vim::around<'"'>, true, true>;
    ciq({});

    auto yiw = vim::change<VimMode::Insert, vim::inner<'"'>, true, false>;
    yiw({});

    selectionFunction({});
}
