// Copyright Mattias Larsson Sköld

#include "visualmode.h"
#include "keys/bufferkeymap.h"
#include "modes/mode.h"
#include "modes/parentmode.h"
#include "script/ienvironment.h"
#include "script/standardcommands.h"
#include "script/vimcommands.h"
#include "text/cursorops.h"
#include "views/editor.h"

std::shared_ptr<IMode> createVisualMode(bool isBlockSelection) {
    auto &sc = StandardCommands::get();

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
            {{"o"}, {sc.toggle_visual_end}},
        },
    };

    if (isBlockSelection) {
        map.bind({{"y"}, {sc.combine(sc.yank_block, sc.normal_mode)}});
    }
    else {
        map.bind({{"y"}, {sc.combine(sc.yank, sc.normal_mode)}});
    }

    auto vimCommand = [](std::shared_ptr<IEnvironment> env) {
        auto &editor = env->editor();
        auto &mode = editor.mode();
        auto motion = getMotion(mode.buffer(), VimCommandType::Visual);

        if (motion) {
            auto cursor = editor.cursor();

            cursor = motion.f(cursor, mode.repetitions());
            editor.cursor(cursor);
        }
    };

    map.defaultAction(vimCommand);

    auto bufferMap = BufferKeyMap{BufferKeyMap::MapType{
        //        {{"iw"}, {sc.select_inner_word}},
        //        {{"aw"}, {sc.select_inner_word}},
        //        {{"i("}, {sc.select_inner_paren}},
        //        {{"a("}, {sc.select_around_paren}},
    }};

    bufferMap.customMatchFunction([](FStringView str) -> BufferKeyMap::ReturnT {
        /// This function matches specific i and a commands. For the motion,
        /// look above

        if (str.empty()) {
            return {BufferKeyMap::NoMatch, {}};
        }
        if (str.front() == 'i' || str.front() == 'a') {
            auto selection = getSelectionFunction(str, VimCommandType::Visual);
            if (selection.match == vim::MatchType::PartialMatch) {
                return {BufferKeyMap::PartialMatch, {}};
            }
            if (selection.match == vim::MatchType::Match) {
                auto wrapper = [selectionFunction = selection.f](
                                   std::shared_ptr<IEnvironment> env) {
                    auto &editor = env->editor();
                    auto num = editor.mode().repetitions();
                    auto cursor = editor.cursor();
                    auto [selection, newCursor] =
                        selectionFunction(cursor, num);

                    if (!selection.empty()) {
                        selection.endPosition(::left(selection.end(), true));
                    }
                    editor.selection(selection);
                };

                return {BufferKeyMap::Match, wrapper};
            }
        }

        /// Separate handling for motions
        auto motion = getMotion(str, VimCommandType::Visual);
        if (motion.match == vim::MatchType::PartialMatch) {
            return {BufferKeyMap::PartialMatch, {}};
        }
        if (motion.match == vim::MatchType::Match) {
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
    //    bufferMap.customMatchFunction([](FStringView str) ->
    //    BufferKeyMap::ReturnT {
    //        auto motion = getMotion(str);
    //        if (motion.match == vim::MatchType::PartialMatch) {
    //            return {BufferKeyMap::PartialMatch, {}};
    //        }
    //        if (motion.match == vim::MatchType::Match) {
    //            if (motion) {
    //                auto wrapper =
    //                    [motion = motion.f](std::shared_ptr<IEnvironment> env)
    //                    {
    //                        auto &editor = env->editor();
    //                        auto num = editor.mode().repetitions();
    //                        auto cursor = editor.cursor();
    //                        cursor = motion(cursor, num);
    //                        editor.cursor(cursor);
    //                    };

    //                return {BufferKeyMap::Match, wrapper};
    //            }
    //        }

    //        return {BufferKeyMap::NoMatch, {}};
    //    });

    auto mode =
        std::make_shared<Mode>(isBlockSelection ? "visual block" : "visual",
                               std::move(map),
                               createParentMode());

    mode->bufferMap(std::move(bufferMap));
    mode->isBlockSelection(isBlockSelection);

    mode->startCallback([](Editor &e) { e.selectionAnchor(e.cursor()); });
    mode->exitCallback([](Editor &e) { e.clearSelection(); });

    mode->shouldEnableNumbers(true);

    return mode;
}
