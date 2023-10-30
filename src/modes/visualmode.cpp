// Copyright Mattias Larsson Sköld

#include "visualmode.h"
#include "modes/mode.h"
#include "modes/parentmode.h"
#include "script/ienvironment.h"
#include "script/standardcommands.h"
#include "script/vimcommands.h"
#include "views/editor.h"

std::shared_ptr<IMode> createVisualMode(bool blockSelection) {
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
            {{Key::Delete}, {sc.combine(sc.erase, sc.normal_mode)}},
            {{"x"}, {sc.combine(sc.erase, sc.normal_mode)}},
            {{"d"}, {sc.combine(sc.erase, sc.normal_mode)}},
            {{"c"}, {sc.combine(sc.erase, sc.insert_mode)}},
            {{"v"}, {sc.visual_mode}},
            {{"V"}, {sc.visual_block_mode}},
            {{Key::Escape}, {sc.normal_mode}},
            {{"\n"}, {sc.down}},
            {{Key::Space}, {sc.right}},

            {{"b"}, {sc.combine(sc.left, sc.word_begin)}},
            {{"e"}, {sc.combine(sc.right, sc.word_end)}},
            {{"w"},
             {sc.combine(sc.word_end, sc.right, sc.word_end, sc.word_begin)}},
            {{">"}, {sc.indent}},
            {{"<"}, {sc.deindent}},
        },
    };

    if (blockSelection) {
        map.bind({{"y"}, {sc.combine(sc.yank_block, sc.normal_mode)}});
    }
    else {
        map.bind({{"y"}, {sc.combine(sc.yank, sc.normal_mode)}});
    }
    map.defaultAction({});

    //    using select = vim::select;

    auto bufferMap = BufferKeyMap{BufferKeyMap::MapType{
        {{"iw"}, {sc.select_inner_word}},
        {{"aw"}, {sc.select_inner_word}},
        {{"i("}, {sc.select_inner_paren}},
        {{"a("}, {sc.select_around_paren}},
    }};

    auto mode =
        std::make_shared<Mode>("visual", std::move(map), createParentMode());

    mode->bufferMap(std::move(bufferMap));
    mode->isBlockSelection(blockSelection);

    mode->startCallback([](Editor &e) { e.anchor(e.cursor()); });
    mode->exitCallback([](Editor &e) { e.clearSelection(); });

    mode->shouldEnableNumbers(true);

    return mode;
}
