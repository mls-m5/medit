// Copyright Mattias Larsson Sköld

#include "parentmode.h"
#include "mode.h"

std::unique_ptr<IMode> createParentMode() {
    auto map = KeyMap{{
        {KeyEvent{Key::KeyCombination, 'S', Modifiers::Ctrl},
         "editor.format\neditor.save"},
        {KeyEvent{Key::KeyCombination, 'B', Modifiers::Ctrl},
         "editor.format\neditor.save\neditor.build"},
        {KeyEvent{Key::KeyCombination, 'W', Modifiers::Ctrl}, "quit"},
    }};

    map.defaultAction("");

    return std::make_unique<Mode>("parent", std::move(map));
}
