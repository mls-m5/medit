
#include "insertmode.h"
#include "modes/mode.h"
#include "modes/parentmode.h"
#include "screen/cursorstyle.h"
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
        {{Key::Escape}, {sc.normal_mode}},
        {KeyEvent{Key::KeyCombination, '\b', Modifiers::Ctrl},
         {sc.erase_before_word}},
        {{"\n"}, {sc.combine(sc.split, sc.copy_indentation)}},
    }};
    map.defaultAction({sc.insert});

    auto mode =
        std::make_shared<Mode>("insert", std::move(map), createParentMode());

    mode->cursorStyle(CursorStyle::Beam);

    return mode;
}
