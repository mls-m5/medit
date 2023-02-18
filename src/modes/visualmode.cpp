// Copyright Mattias Larsson Sköld

#include "visualmode.h"
#include "modes/mode.h"
#include "modes/parentmode.h"
#include "script/ienvironment.h"
#include "views/editor.h"

std::shared_ptr<IMode> createVisualMode(IEnvironment &env,
                                        bool blockSelection) {
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
            {{Key::Backspace}, {"editor.left"}},
            {{"X"}, {"editor.erase"}},
            {{Key::Delete}, {"neditor.erase\neditor.normalmode"}},
            {{"x"}, {"editor.erase\neditor.normalmode"}},
            {{"d"}, {"editor.erase\neditor.normalmode"}},
            {{"c"}, {"editor.erase\neditor.insertmode"}},
            {{"V"}, {"editor.visualblockmode"}},
            {{"v"}, {"editor.visualmode"}},
            {{Key::Escape}, {"editor.normalmode"}},
            //            {{Key::Return}, {"editor.down"}},
            {{"\n"}, {"editor.down"}},
            {{Key::Space}, {"editor.right"}},

            {{"b"}, {"editor.left\neditor.word_begin"}},
            {{"e"}, {"editor.right\neditor.word_end"}},
            {{"w"},
             {"editor.word_end\neditor.right\neditor.word_end\neditor.word_"
              "begin"}},
            {{"y"}, {"editor.yank\neditor.normalmode"}},

        },
    };
    map.defaultAction({});

    auto bufferMap = BufferKeyMap{BufferKeyMap::MapType{
        {{"iw"}, {"editor.select_inner_word"}},
        {{"aw"}, {"editor.select_inner_word"}},
    }};

    auto mode = std::make_shared<Mode>("visual",
                                       std::move(map),
                                       CursorStyle::Block,
                                       createParentMode(),
                                       std::move(bufferMap),
                                       blockSelection);

    mode->startCallback([](Editor &e) { e.anchor(e.cursor()); });
    mode->exitCallback([](Editor &e) { e.clearSelection(); });

    return mode;
}
