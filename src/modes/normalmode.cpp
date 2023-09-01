// Copyright Mattias Larsson Sköld

#include "normalmode.h"
#include "modes/mode.h"
#include "modes/parentmode.h"
#include "script/standardcommands.h"
#include "views/mainwindow.h"

std::shared_ptr<IMode> createNormalMode() {
    using Ptr = StandardCommands::EnvPtrT;
    auto &sc = StandardCommands::get();

    auto map = KeyMap{
        {
            {{Key::Left}, sc.left},
            {{Key::Right}, sc.right},
            {{Key::Down}, sc.down},
            {{Key::Up}, sc.up},
            {{"h"}, sc.left},
            {{"l"}, sc.right},
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

    auto bufferMap = BufferKeyMap{BufferKeyMap::MapType{
        {{"dd"}, sc.delete_line},
        {{"dw"}, sc.combine(sc.select_word, sc.erase)},
        {{"diw"}, sc.combine(sc.select_inner_word, sc.erase)},

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

    return std::make_shared<Mode>("normal",
                                  std::move(map),
                                  CursorStyle::Block,
                                  createParentMode(),
                                  std::move(bufferMap));
}
