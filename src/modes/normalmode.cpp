// Copyright Mattias Larsson Sköld

#include "normalmode.h"
#include "modes/mode.h"
#include "modes/parentmode.h"

std::unique_ptr<IMode> createNormalMode() {
    auto map = KeyMap{
        {
            {{Key::Left}, {"editor.left"}},
            {{Key::Right}, {"editor.right"}},
            {{Key::Down}, {"editor.down"}},
            {{Key::Up}, {"editor.up"}},
            {{"h"}, {"editor.left"}},
            {{"l"}, {"editor.right"}},
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
            {{Key::Return}, {"editor.down"}},
            {{"\n"}, {"editor.down"}},
            {{Key::Space}, {"editor.right"}},
            {{"i"}, {"editor.insertmode"}},
            {{"v"}, {"editor.visualmode"}},
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
        {{"dw"}, {"editor.delete_word"}},
        {{"diw"}, {"editor.delete_iw"}},

        {{"yy"}, {"editor.yank_line\neditor.normalmode"}},
        {{"yw"}, {"editor.yank_word\neditor.normalmode"}},
        {{"yiw"}, {"editor.yank_iw\neditor.normalmode"}},

        {{"cc"},
         {"editor.delete_line\neditor.split\neditor.left\neditor.insertmode"}},
        {{"cw"}, {"editor.delete_word\neditor.insertmode"}},
        {{"ciw"}, {"editor.delete_iw\neditor.insertmode"}},
    }};

    return std::make_unique<Mode>(
        "normal", std::move(map), createParentMode(), std::move(bufferMap));
}
