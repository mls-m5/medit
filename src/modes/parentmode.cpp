// Copyright Mattias Larsson Sköld

#include "parentmode.h"
#include "mode.h"
#include "script/standardcommands.h"
#include "views/mainwindow.h"

std::shared_ptr<IMode> createParentMode() {
    auto &sc = StandardCommands::get();
    using Ptr = StandardCommands::EnvPtrT;
    auto map = KeyMap{{
        {KeyEvent{Key::F2},
         {[](Ptr env) { env->mainWindow().gotoDefinition(); }}},
        {KeyEvent{Key::F4}, sc.switch_header},
        {KeyEvent{Key::F5}, sc.run},
        {KeyEvent{Key::Home}, {sc.home}},
        {KeyEvent{Key::End}, {sc.end}},
        {KeyEvent{Key::PageUp}, {sc.page_up}},
        {KeyEvent{Key::PageDown}, {sc.page_down}},
        {KeyEvent{Key::KeyCombination, 'T', Modifiers::Ctrl}, {[](Ptr env) {
             env->mainWindow().showConsole(env->mainWindow().currentConsole());
         }}},
        {KeyEvent{Key::KeyCombination, 'E', Modifiers::Ctrl},
         {[](Ptr env) { env->mainWindow().switchEditor(); }}},
        {KeyEvent{Key::KeyCombination, '7', Modifiers::Ctrl},
         sc.toggle_comment},
        {KeyEvent{Key::KeyCombination, 'O', Modifiers::Ctrl},
         {[](Ptr env) { env->mainWindow().showOpen(); }}},
        {KeyEvent{Key::KeyCombination, 'S', Modifiers::Ctrl},
         sc.combine(sc.format, sc.save)},
        {KeyEvent{Key::KeyCombination, 'C', Modifiers::Ctrl}, sc.copy},
        {KeyEvent{Key::KeyCombination, 'X', Modifiers::Ctrl}, sc.cut},
        {KeyEvent{Key::KeyCombination, 'Z', Modifiers::Ctrl}, sc.undo},
        {KeyEvent{Key::KeyCombination, 'Y', Modifiers::Ctrl}, sc.redo},
        {KeyEvent{Key::KeyCombination, 'B', Modifiers::Ctrl},
         sc.combine(sc.save, sc.build)},
        {KeyEvent{Key::KeyCombination, 'W', Modifiers::Ctrl}, sc.close_buffer},
        {KeyEvent{Key::KeyCombination, 'K', Modifiers::Ctrl},
         {[](Ptr env) { env->mainWindow().showLocator(); }}},
        {KeyEvent{Key::KeyCombination, ' ', Modifiers::Ctrl},
         {[](Ptr env) { env->mainWindow().autoComplete(); }}},
        {KeyEvent{Key::KeyCombination, 'R', Modifiers::Ctrl}, sc.rename_symbol},
        {KeyEvent{Key::KeyCombination, 'A', Modifiers::Ctrl}, sc.select_all},
        {KeyEvent{Key::KeyCombination, 'P', Modifiers::Ctrl},
         sc.command_palette},
    }};

    map.defaultAction({});

    return std::make_shared<Mode>("parent", std::move(map), CursorStyle::Block);
}
