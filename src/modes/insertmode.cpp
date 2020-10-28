
#include "insertmode.h"
#include "modes/mode.h"

std::unique_ptr<IMode> createInsertMode() {
    auto map = KeyMap{{
        {{Key::Left}, "editor.previous"},
        {{Key::Right}, "editor.next"},
        {{Key::Down}, "editor.down"},
        {{Key::Up}, "editor.up"},
        {{Key::Backspace}, "editor.erase"},
        {{Key::Escape}, "editor.normalmode"},
    }};
    map.defaultAction("editor.insert");

    return std::make_unique<Mode>("insert", std::move(map));
}
