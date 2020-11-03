// Copyright Mattias Larsson Sköld

#include "normalmode.h"
#include "modes/mode.h"
#include "modes/parentmode.h"

std::unique_ptr<IMode> createNormalMode() {
    auto map = KeyMap{{
        {{Key::Left}, "editor.left"},
        {{Key::Right}, "editor.right"},
        {{Key::Down}, "editor.down"},
        {{Key::Up}, "editor.up"},
        {{"h"}, "editor.left"},
        {{"l"}, "editor.right"},
        {{"j"}, "editor.down"},
        {{"k"}, "editor.up"},
        {{"J"}, "editor.join"},
        {{"o"}, "editor.end\neditor.split\neditor.insertmode"},
        {{"O"}, "editor.home\neditor.split\neditor.insertmode\neditor.up"},
        {{Key::Backspace}, "editor.left"},
        {{"X"}, "editor.left"},
        {{Key::Delete}, "editor.right\neditor.erase"},
        {{"x"}, "editor.right\neditor.erase"},
        {{Key::Escape}, "editor.normalmode"},
        {{Key::Return}, "editor.down"},
        {{Key::Space}, "editor.right"},
        {{"i"}, "editor.insertmode"},
        {{"I"}, "editor.home\neditor.insertmode"},
        {{"a"}, "editor.right\neditor.insertmode"},
        {{"A"}, "editor.end\neditor.insertmode"},
    }};
    map.defaultAction("");

    return std::make_unique<Mode>("normal", std::move(map), createParentMode());
}
