// Copyright Mattias Larsson Sköld

#include "visualmode.h"
#include "modes/mode.h"
#include "modes/parentmode.h"
#include "script/ienvironment.h"
#include "script/standardcommands.h"
#include "views/editor.h"

std::shared_ptr<IMode> createVisualMode(IEnvironment &env,
                                        bool blockSelection) {
    using Ptr = StandardCommands::EnvPtrT;
    auto &sc = StandardCommands::get();

    auto map = KeyMap{
        {
            {{Key::Left}, {sc.left}},
            {{Key::Right}, {sc.right}},
            {{Key::Down}, {sc.down}},
            {{Key::Up}, {sc.up}},
            {{"h"}, {sc.left}},
            {{"l"}, {sc.right}},
            {{"j"}, {sc.down}},
            {{"k"}, {sc.up}},
            {{Key::Backspace}, {sc.left}},
            {{"X"}, {sc.erase}},
            {{Key::Delete},
             {
                 sc.combine(sc.erase, sc.normalMode)
                 //            "neditor.erase\neditor.normalmode"
             }},
            {{"x"},
             {
                 sc.combine(sc.erase, sc.normalMode)
                 //            "editor.erase\neditor.normalmode"
             }},
            {{"d"},
             {
                 sc.combine(sc.erase, sc.normalMode)
                 //            "editor.erase\neditor.normalmode"
             }},
            {{"c"},
             {
                 sc.combine(sc.erase, sc.insertMode)
                 //            "editor.erase\neditor.insertmode"
             }},
            {{"v"},
             {
                 sc.visualMode
                 //            "editor.visualmode"
             }},
            {{"V"},
             {
                 sc.visualBlockMode
                 //            "editor.visualblockmode"
             }},
            {{Key::Escape},
             {
                 sc.normalMode
                 //            "editor.normalmode"
             }},
            //            {{Key::Return}, {"editor.down"}},
            {{"\n"},
             {
                 sc.down
                 //            "editor.down"
             }},
            {{Key::Space},
             {
                 sc.right
                 //            "editor.right"
             }},

            {{"b"},
             {
                 sc.combine(sc.left, sc.wordBegin)
                 //            "editor.left\neditor.word_begin"
             }},
            {{"e"},
             {
                 sc.combine(sc.right, sc.wordEnd)
                 //            "editor.right\neditor.word_end"
             }},
            {{"w"},
             {
                 sc.combine(sc.wordEnd, sc.right, sc.wordEnd, sc.wordBegin)
                 //            "editor.word_end\neditor.right\neditor.word_end\neditor.word_"
                 //              "begin"
             }},
            {{"y"},
             {
                 sc.combine(sc.yank, sc.normalMode)
                 //            "editor.yank\neditor.normalmode"
             }},

        },
    };
    map.defaultAction({});

    auto bufferMap = BufferKeyMap{BufferKeyMap::MapType{
        {{"iw"}, {sc.selectInnerWord}},
        {{"aw"}, {sc.selectInnerWord}},
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
