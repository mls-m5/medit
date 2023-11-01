// Copyright Mattias Larsson Sköld

#include "visualmode.h"
#include "modes/mode.h"
#include "modes/parentmode.h"
#include "script/ienvironment.h"
#include "script/standardcommands.h"
#include "script/vimcommands.h"
#include "views/editor.h"

std::shared_ptr<IMode> createVisualMode(bool isBlockSelection) {
    auto &sc = StandardCommands::get();

    auto vimCommand = [](std::shared_ptr<IEnvironment> env) {
        auto &editor = env->editor();
        auto &mode = editor.mode();
        auto motion = getMotion(mode.buffer());

        if (!motion) {
            return; /// Failed
        }

        auto cursor = editor.cursor();

        cursor = motion.f(cursor, mode.repetitions());
        editor.cursor(cursor);
    };

    auto map = KeyMap{
        {
            {{Key::Left}, {sc.left}},
            {{Key::Right}, {sc.right}},
            {{Key::Down}, {sc.down}},
            {{Key::Up}, {sc.up}},
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
            {{">"}, {sc.indent}},
            {{"<"}, {sc.deindent}},
        },
    };

    if (isBlockSelection) {
        map.bind({{"y"}, {sc.combine(sc.yank_block, sc.normal_mode)}});
    }
    else {
        map.bind({{"y"}, {sc.combine(sc.yank, sc.normal_mode)}});
    }

    map.defaultAction(vimCommand);

    auto bufferMap = BufferKeyMap{BufferKeyMap::MapType{
        {{"iw"}, {sc.select_inner_word}},
        {{"aw"}, {sc.select_inner_word}},
        {{"i("}, {sc.select_inner_paren}},
        {{"a("}, {sc.select_around_paren}},
    }};

    bufferMap.customMatchFunction([](FStringView str) -> BufferKeyMap::ReturnT {
        auto m = getMotion(str);
        if (m.match == vim::MatchType::PartialMatch) {
            return {BufferKeyMap::PartialMatch, {}};
        }
        if (m.match == vim::MatchType::Match) {
            auto motion = getMotion(str);
            if (motion) {
                auto wrapper =
                    [motion = motion.f](std::shared_ptr<IEnvironment> env) {
                        auto &editor = env->editor();
                        auto num = editor.mode().repetitions();
                        auto cursor = editor.cursor();
                        cursor = motion(cursor, num);
                        editor.cursor(cursor);
                    };

                return {BufferKeyMap::Match, wrapper};
            }
        }

        return {BufferKeyMap::NoMatch, {}};
    });

    auto mode =
        std::make_shared<Mode>(isBlockSelection ? "visual block" : "visual",
                               std::move(map),
                               createParentMode());

    mode->bufferMap(std::move(bufferMap));
    mode->isBlockSelection(isBlockSelection);

    mode->startCallback([](Editor &e) { e.anchor(e.cursor()); });
    mode->exitCallback([](Editor &e) { e.clearSelection(); });

    mode->shouldEnableNumbers(true);

    return mode;
}
