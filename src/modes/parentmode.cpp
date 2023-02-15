// Copyright Mattias Larsson Sköld

#include "parentmode.h"
#include "mode.h"

std::shared_ptr<IMode> createParentMode() {
    auto map = KeyMap{{
        {KeyEvent{Key::F2}, {"editor.goto_definition"}},
        {KeyEvent{Key::F4}, {"editor.switch_header"}},
        {KeyEvent{Key::F5}, {"editor.run"}},
        {KeyEvent{Key::Home}, {"editor.home"}},
        {KeyEvent{Key::End}, {"editor.end"}},
        {KeyEvent{Key::PageUp}, {"editor.page_up"}},
        {KeyEvent{Key::PageDown}, {"editor.page_down"}},
        {KeyEvent{Key::KeyCombination, 'T', Modifiers::Ctrl}, {"show_console"}},
        {KeyEvent{Key::KeyCombination, 'E', Modifiers::Ctrl},
         {"switch_editor"}},
        {KeyEvent{Key::KeyCombination, '7', Modifiers::Ctrl},
         {"editor.toggle_comment"}},
        {KeyEvent{Key::KeyCombination, 'O', Modifiers::Ctrl},
         {"editor.show_open"}},
        {KeyEvent{Key::KeyCombination, 'S', Modifiers::Ctrl},
         {"editor.format\neditor.save"}},
        {KeyEvent{Key::KeyCombination, 'C', Modifiers::Ctrl},
         {"editor.format\neditor.copy"}},
        {KeyEvent{Key::KeyCombination, 'X', Modifiers::Ctrl},
         {"editor.format\neditor.cut"}},
        {KeyEvent{Key::KeyCombination, 'Z', Modifiers::Ctrl}, {"editor.undo"}},
        {KeyEvent{Key::KeyCombination, 'Y', Modifiers::Ctrl}, {"editor.redo"}},
        {KeyEvent{Key::KeyCombination, 'B', Modifiers::Ctrl},
         {"editor.save\neditor.build"}},
        {KeyEvent{Key::KeyCombination, 'W', Modifiers::Ctrl}, {"quit"}},
        {KeyEvent{Key::KeyCombination, 'K', Modifiers::Ctrl},
         {"window.show_locator"}},
        {KeyEvent{Key::KeyCombination, ' ', Modifiers::Ctrl},
         {"editor.auto_complete"}},

    }};

    map.defaultAction({});

    return std::make_shared<Mode>("parent", std::move(map), CursorStyle::Block);
}
