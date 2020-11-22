
#include "insertmode.h"
#include "modes/mode.h"
#include "modes/parentmode.h"

std::shared_ptr<IMode> createInsertMode() {
    auto map = KeyMap{{
        {{Key::Left}, {"editor.left"}},
        {{Key::Right}, {"editor.right"}},
        {{Key::Down}, {"editor.down"}},
        {{Key::Up}, {"editor.up"}},
        {{Key::Backspace}, {"editor.erase"}},
        {{Key::Delete}, {"editor.right\neditor.erase"}},
        {{Key::Escape}, {"editor.normalmode"}},
        {{Key::KeyCombination}, {}},
        {{"\n"}, {"editor.split\neditor.copyindentation"}},
    }};
    map.defaultAction({"editor.insert"});

    return std::make_shared<Mode>(
        "insert", std::move(map), CursorStyle::Beam, createParentMode());
}
