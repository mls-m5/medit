// Copyright Mattias Larsson Sköld

#include "normalmode.h"
#include "modes/mode.h"
#include "modes/parentmode.h"
#include "screen/cursorstyle.h"
#include "script/ienvironment.h"
#include "script/standardcommands.h"
#include "script/vimcommands.h"
#include "views/mainwindow.h"
#include <memory>

std::shared_ptr<IMode> createNormalMode() {
    using Ptr = StandardCommands::EnvPtrT;
    auto &sc = StandardCommands::get();

    auto vimMotion = [](std::shared_ptr<IEnvironment> env) {
        auto &editor = env->editor();
        auto &mode = editor.mode();
        auto motion = getMotion(mode.buffer());

        if (!motion) {
            return; /// Failed
        }

        auto cursor = editor.cursor();

        cursor = (*motion)(cursor, mode.repetitions());
        editor.cursor(cursor);
    };

    auto map = KeyMap{
        {
            {{Key::Left}, sc.left},
            {{Key::Right}, sc.right},
            {{Key::Down}, sc.down},
            {{Key::Up}, sc.up},
            {{"h"}, vimMotion},
            {{"l"}, vimMotion},
            //            {{"h"}, sc.left},
            //            {{"l"}, sc.right},
            {{"j"}, sc.down},
            {{"k"}, sc.up},
            {{"J"}, sc.join},
            {{"p"}, sc.paste},
            {{"P"}, sc.paste_before},
            {{"o"},
             sc.combine(sc.end,
                        sc.split,
                        sc.copy,
                        sc.copy_indentation,
                        sc.insert_mode)},
            {{"O"},
             {sc.combine(sc.home,
                         sc.split,
                         sc.insert_mode,
                         sc.up,
                         sc.copy_indentation)}},
            {{Key::Backspace}, {sc.left}},
            {{"X"}, {sc.erase}},
            {{Key::Delete}, sc.combine(sc.right, sc.erase)},
            {{"x"}, sc.combine(sc.right, sc.erase)},
            {{Key::Escape}, {[](Ptr env) { env->mainWindow().escape(); }}},
            {{"\n"}, {sc.down}},
            {{Key::Space}, {sc.right}},
            {{"i"}, sc.insert_mode},
            {{"v"}, sc.visual_mode},
            {{"V"}, sc.visual_block_mode},
            {{"u"}, sc.undo_major},
            {{"U"}, sc.redo},
            {{"I"}, sc.combine(sc.home, sc.insert_mode)},
            {{"a"}, sc.combine(sc.right, sc.insert_mode)},
            {{"A"}, sc.combine(sc.end, sc.insert_mode)},
            {{"b"}, sc.combine(sc.left, sc.word_begin)},
            {{"e"}, sc.combine(sc.right, sc.word_end)},
            {{"w"},
             sc.combine(sc.word_end, sc.right, sc.word_end, sc.word_begin)},
            {{">"}, {sc.indent}},
            {{"<"}, {sc.deindent}},
        },
    };
    map.defaultAction({});

    auto action = createVimAction(VimMode::Normal);

    auto bufferMap = BufferKeyMap{BufferKeyMap::MapType{
        {{"dd"}, sc.delete_line},
        //        {{"dw"}, sc.combine(sc.select_word, sc.erase)},
        {{"dw"}, action},
        {{"diw"}, action},
        //        {{"diw"}, sc.combine(sc.select_inner_word, sc.erase)},

        {{"cc"}, sc.combine(sc.clear_line, sc.copy, sc.copy_indentation)},
        {{"cw"}, sc.combine(sc.select_word, sc.erase, sc.insert_mode)},
        {{"ciw"}, sc.combine(sc.select_inner_word, sc.erase, sc.insert_mode)},

        {{"yy"}, sc.combine(sc.yank_line, sc.normal_mode)},
        {{"yw"}, sc.combine(sc.select_word, sc.yank, sc.normal_mode)},
        {{"yiw"}, sc.combine(sc.select_inner_word, sc.yank, sc.normal_mode)},

        {{"cc"}, sc.combine(sc.delete_line, sc.split, sc.left, sc.insert_mode)},
        {{"cw"}, sc.combine(sc.select_word, sc.erase, sc.insert_mode)},
        {{"ciw"}, sc.combine(sc.select_inner_word, sc.erase, sc.insert_mode)},

        {{"gd"}, {[](Ptr env) { env->mainWindow().gotoDefinition(); }}},
    }};

    auto mode =
        std::make_shared<Mode>("normal", std::move(map), createParentMode());

    mode->bufferMap(std::move(bufferMap));
    mode->shouldEnableNumbers(true);

    return mode;
}
