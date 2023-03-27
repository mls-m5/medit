
#include "insertmode.h"
#include "modes/mode.h"
#include "modes/parentmode.h"
#include "script/standardcommands.h"

std::shared_ptr<IMode> createInsertMode() {
    using Ptr = StandardCommands::EnvPtrT;
    auto &sc = StandardCommands::get();

    auto map = KeyMap{{
        {{Key::Left}, {sc.left}},
        {{Key::Right}, {sc.right}},
        {{Key::Down}, {sc.down}},
        {{Key::Up}, {sc.up}},
        {{Key::Backspace}, {sc.erase}},
        {{Key::Delete}, {sc.combine(sc.right, sc.erase)}},
        {{Key::Escape}, {sc.normalMode}},
        {KeyEvent{Key::KeyCombination, '\b', Modifiers::Ctrl},
         {sc.eraseBeforeWord}},
        {{"\n"}, {sc.combine(sc.split, sc.copyIndentation)}},
    }};
    map.defaultAction({sc.insert});

    return std::make_shared<Mode>(
        "insert", std::move(map), CursorStyle::Beam, createParentMode());
}
