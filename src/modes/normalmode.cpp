
#include "normalmode.h"
#include "modes/mode.h"

std::unique_ptr<IMode> createNormalMode() {
    auto map = KeyMap{{
        {{"h"}, "editor.previous"},
        {{"l"}, "editor.next"},
        {{"j"}, "editor.down"},
        {{"k"}, "editor.up"},
        {{Key::Backspace}, "editor.prevous"},
        {{Key::Escape}, "editor.normalmode"},
        {{"i"}, "editor.insertmode"},
    }};
    map.defaultAction("");

    return std::make_unique<Mode>("normal", std::move(map));
}
