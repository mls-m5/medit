// Copyright Mattias Larsson Sköld

#include "normalmode.h"
#include "modes/mode.h"
#include "modes/parentmode.h"
#include "script/standardcommands.h"

std::shared_ptr<IMode> createNormalMode() {
    auto &sc = StandardCommands::get();

    auto map = KeyMap{
        {
            {{Key::Left}, {sc.left}},
            {{Key::Right}, {sc.right}},
            {{Key::Down}, {"editor.down"}},
            {{Key::Up}, {"editor.up"}},
            {{"h"}, {sc.left}},
            {{"l"}, {sc.right}},
            {{"j"}, {"editor.down"}},
            {{"k"}, {"editor.up"}},
            {{"J"}, {"editor.join"}},
            {{"p"}, {"editor.paste"}},
            {{"P"}, {"editor.paste_before"}},
            {{"o"},
             {"editor.end\neditor.split\neditor.copyindentation\neditor."
              "insertmode"}},
            {{"O"},
             {"editor.home\neditor.split\neditor.insertmode\neditor.up\neditor."
              "copyindentation"}},
            {{Key::Backspace}, {"editor.left"}},
            {{"X"}, {"editor.erase"}},
            {{Key::Delete}, {"editor.right\neditor.erase"}},
            {{"x"}, {"editor.right\neditor.erase"}},
            {{Key::Escape}, {"escape"}},
            {{"\n"}, {"editor.down"}},
            {{Key::Space}, {"editor.right"}},
            {{"i"}, {"editor.insertmode"}},
            {{"v"}, {"editor.visualmode"}},
            {{"V"}, {"editor.visualblockmode"}},
            {{"u"}, {"editor.undo_major"}},
            {{"U"}, {"editor.redo"}},
            {{"I"}, {"editor.home\neditor.insertmode"}},
            {{"a"}, {"editor.right\neditor.insertmode"}},
            {{"A"}, {"editor.end\neditor.insertmode"}},

            {{"b"}, {"editor.left\neditor.word_begin"}},
            {{"e"}, {"editor.right\neditor.word_end"}},
            {{"w"},
             {"editor.word_end\neditor.right\neditor.word_end\neditor.word_"
              "begin"}},

        },
    };
    map.defaultAction({});

    auto bufferMap = BufferKeyMap{BufferKeyMap::MapType{
        {{"dd"}, {"editor.delete_line"}},
        {{"dw"}, {"editor.select_word\neditor.erase"}},
        {{"diw"}, {"editor.select_inner_word\neditor.erase"}},

        {{"cc"}, {"editor.clear_line\neditor.copyindentation"}},
        {{"cw"}, {"editor.select_word\neditor.erase\neditor.insertmode"}},
        {{"ciw"},
         {"editor.select_inner_word\neditor.erase\neditor.insertmode"}},

        {{"yy"}, {"editor.yank_line\neditor.normalmode"}},
        {{"yw"}, {"editor.select_word\neditor.yank\neditor.normalmode"}},
        {{"yiw"}, {"editor.select_inner_word\neditor.yank\neditor.normalmode"}},

        {{"cc"},
         {"editor.delete_line\neditor.split\neditor.left\neditor.insertmode"}},
        {{"cw"}, {"editor.select_word\neditor.erase\neditor.insertmode"}},
        {{"ciw"},
         {"editor.select_inner_word\neditor.erase\neditor.insertmode"}},

        {{"gd"}, {"editor.goto_definition"}},
    }};

    return std::make_shared<Mode>("normal",
                                  std::move(map),
                                  CursorStyle::Block,
                                  createParentMode(),
                                  std::move(bufferMap));
}
