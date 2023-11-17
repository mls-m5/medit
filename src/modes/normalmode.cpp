// Copyright Mattias Larsson Sköld

#include "normalmode.h"
#include "keys/bufferkeymap.h"
#include "modes/mode.h"
#include "modes/parentmode.h"
#include "script/ienvironment.h"
#include "script/standardcommands.h"
#include "script/vimcommands.h"
#include "text/fstringview.h"
#include "views/mainwindow.h"
#include <memory>

std::shared_ptr<IMode> createNormalMode() {
    using Ptr = StandardCommands::EnvPtrT;
    auto &sc = StandardCommands::get();

    auto map = KeyMap{
        {
            {{Key::Left}, sc.left},
            {{Key::Right}, sc.right},
            {{Key::Down}, sc.down},
            {{Key::Up}, sc.up},
            {{"J"}, sc.join},
            {{"p"}, sc.paste},
            {{"P"}, sc.paste_before},
            {{"o"},
             sc.combine(sc.insert_mode, sc.end, sc.split, sc.copy_indentation)},
            {{"O"},
             {sc.combine(sc.home,
                         sc.split,
                         sc.insert_mode,
                         sc.up,
                         sc.copy_indentation)}},
            {{Key::Backspace}, {sc.left}},
            {{"X"}, sc.erase},
            {{Key::Delete}, sc.combine(sc.right, sc.erase)},
            {{"x"}, sc.erase_after},
            {{Key::Escape}, {[](Ptr env) { env->mainWindow().escape(); }}},
            {{"\n"}, {sc.down}},
            {{Key::Space}, {sc.right}},
            {{"i"}, sc.insert_mode},
            {{"v"}, sc.visual_mode},
            {{"V"}, sc.visual_block_mode},
            {{"u"}, sc.undo_major},
            {{"U"}, sc.redo},
            {{"I"}, sc.combine(sc.home, sc.insert_mode)},
            {{"a"}, sc.combine(sc.insert_mode, sc.right)},
            {{"A"}, sc.combine(sc.insert_mode, sc.end)},
            {{"s"}, sc.combine(sc.erase_after, sc.insert_mode)},
            {{">"}, {sc.indent}},
            {{"<"}, {sc.deindent}},

            {{"D"}, {sc.delete_until_end_of_line}},
            {{"C"}, {sc.change_until_end_of_line}},
        },
    };

    auto vimMotion = [](std::shared_ptr<IEnvironment> env) {
        auto &editor = env->editor();
        auto &mode = editor.mode();
        auto motion = getMotion(mode.buffer(), VimCommandType::Motion);

        if (!motion) {
            return;
        }

        auto cursor = editor.cursor();

        cursor = motion.f(cursor, mode.repetitions());
        editor.cursor(cursor);
    };

    map.defaultAction(vimMotion);

    auto bufferMap = BufferKeyMap{BufferKeyMap::MapType{
        {{"dd"}, sc.delete_line},
        {{"dw"}, sc.delete_word}, // Does not use normal w-motion on first

        {{"cc"}, sc.combine(sc.clear_line, sc.copy, sc.copy_indentation)},
        //        {{"cw"}, sc.combine(sc.select_word, sc.erase,
        //        sc.insert_mode)},
        {{"ciw"}, sc.combine(sc.select_inner_word, sc.erase, sc.insert_mode)},

        {{"yy"}, sc.combine(sc.yank_line, sc.normal_mode)},
        {{"yw"}, sc.combine(sc.select_word, sc.yank, sc.normal_mode)},
        {{"yiw"}, sc.combine(sc.select_inner_word, sc.yank, sc.normal_mode)},

        {{"cc"}, sc.combine(sc.delete_line, sc.split, sc.left, sc.insert_mode)},
        {{"cw"}, sc.change_word},
        //        {{"ciw"}, sc.combine(sc.select_inner_word, sc.erase,
        //        sc.insert_mode)},

        {{"gd"}, {[](Ptr env) { env->mainWindow().gotoDefinition(); }}},
    }};

    bufferMap.customMatchFunction([](FStringView str) -> BufferKeyMap::ReturnT {
        auto action = findVimAction(str, VimMode::Normal);
        if (action.match == vim::PartialMatch) {
            return {BufferKeyMap::PartialMatch, {}};
        }

        if (action.match == vim::NoMatch) {
            return {BufferKeyMap::NoMatch, {}};
        }

        return {BufferKeyMap::Match, action.f};
    });

    auto mode =
        std::make_shared<Mode>("normal", std::move(map), createParentMode());

    mode->bufferMap(std::move(bufferMap));
    mode->shouldEnableNumbers(true);

    return mode;
}
