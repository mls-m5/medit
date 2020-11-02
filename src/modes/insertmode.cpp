
#include "insertmode.h"
#include "modes/mode.h"
#include "modes/parentmode.h"

std::unique_ptr<IMode> createInsertMode() {
    auto map = KeyMap{{
        {{Key::Left}, "editor.left"},
        {{Key::Right}, "editor.right"},
        {{Key::Down}, "editor.down"},
        {{Key::Up}, "editor.up"},
        {{Key::Backspace}, "editor.erase"},
        {{Key::Escape}, "editor.normalmode"},
        {{Key::KeyCombination}, ""},
    }};
    map.defaultAction("editor.insert");

    return std::make_unique<Mode>("insert", std::move(map), createParentMode());
}
