
#include "normalmode.h"
#include "modes/mode.h"

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
        {{Key::Backspace}, "editor.prevous"},
        {{Key::Escape}, "editor.normalmode"},
        {{Key::Return}, "editor.down"},
        {{Key::Space}, "editor.right"},
        {{"i"}, "editor.insertmode"},
    }};
    map.defaultAction("");

    return std::make_unique<Mode>("normal", std::move(map));
}
