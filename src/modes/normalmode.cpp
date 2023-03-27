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
            {{"P"}, sc.pasteBefore},
            {{"o"},
             sc.combine(
                 sc.end, sc.split, sc.copy, sc.copyIndentation, sc.insertMode)},
            {{"O"},
             {sc.combine(
                 sc.home, sc.split, sc.insertMode, sc.up, sc.copyIndentation)}},
            {{Key::Backspace}, {sc.left}},
            {{"X"}, {sc.erase}},
            {{Key::Delete}, sc.combine(sc.right, sc.erase)},
            {{"x"}, sc.combine(sc.right, sc.erase)},
            {{Key::Escape}, {[](Ptr env) { env->mainWindow().escape(); }}},
            {{"\n"}, {sc.down}},
            {{Key::Space}, {sc.right}},
            {{"i"}, sc.insertMode},
            {{"v"}, sc.visualMode},
            {{"V"}, sc.visualBlockMode},
            {{"u"}, sc.undoMajor},
            {{"U"}, sc.redo},
            {{"I"}, sc.combine(sc.home, sc.insertMode)},
            {{"a"}, sc.combine(sc.right, sc.insertMode)},
            {{"A"}, sc.combine(sc.end, sc.insertMode)},
            {{"b"}, sc.combine(sc.left, sc.wordBegin)},
            {{"e"}, sc.combine(sc.right, sc.wordEnd)},
            {{"w"}, sc.combine(sc.wordEnd, sc.right, sc.wordEnd, sc.wordBegin)},
        },
    };
    map.defaultAction({});

    auto bufferMap = BufferKeyMap{BufferKeyMap::MapType{
        {{"dd"}, sc.deleteLine},
        {{"dw"}, sc.combine(sc.selectWord, sc.erase)},
        {{"diw"}, sc.combine(sc.selectInnerWord, sc.erase)},

        {{"cc"}, sc.combine(sc.clearLine, sc.copy, sc.copyIndentation)},
        {{"cw"}, sc.combine(sc.selectWord, sc.erase, sc.insertMode)},
        {{"ciw"}, sc.combine(sc.selectInnerWord, sc.erase, sc.insertMode)},

        {{"yy"}, sc.combine(sc.yankLine, sc.normalMode)},
        {{"yw"}, sc.combine(sc.selectWord, sc.yank, sc.normalMode)},
        {{"yiw"}, sc.combine(sc.selectInnerWord, sc.yank, sc.normalMode)},

        {{"cc"}, sc.combine(sc.deleteLine, sc.split, sc.left, sc.insertMode)},
        {{"cw"}, sc.combine(sc.selectWord, sc.erase, sc.insertMode)},
        {{"ciw"}, sc.combine(sc.selectInnerWord, sc.erase, sc.insertMode)},

        {{"gd"}, {[](Ptr env) { env->mainWindow().gotoDefinition(); }}},
    }};

    return std::make_shared<Mode>("normal",
                                  std::move(map),
                                  CursorStyle::Block,
                                  createParentMode(),
                                  std::move(bufferMap));
}
