// Copyright Mattias Larsson Sköld

#include "parentmode.h"
#include "keys/event.h"
#include "mode.h"
#include "script/standardcommands.h"
#include "views/mainwindow.h"

std::shared_ptr<IMode> createParentMode() {
    auto &sc = StandardCommands::get();
    using Ptr = StandardCommands::EnvPtrT;
    // Binds `key` to `f` as before, and shows `key` as the shortcut of the
    // command `name` (e.g. in the command palette). Only for display.
    auto named = [&sc](KeyEvent key, CommandT f, const std::string &name) {
        sc.shortcut(name, key);
        return std::pair<KeyMatch, CommandT>{key, std::move(f)};
    };
    auto map = KeyMap{{
        {KeyEvent{Key::F2},
         {[](Ptr env) { env->mainWindow().gotoDefinition(); }}},
        named(KeyEvent{Key::F4}, sc.switch_header, "switch_header"),
        named(KeyEvent{Key::F5}, sc.debug_run, "debug_run"),
        named(KeyEvent{Key::F9},
              sc.debug_toggle_breakpoint,
              "debug_toggle_breakpoint"),
        named(KeyEvent{Key::F10}, sc.debug_step_over, "debug_step_over"),
        named(KeyEvent{Key::F11}, sc.debug_step_into, "debug_step_into"),
        //        {KeyEvent{Key::F11, {}, Modifiers::Ctrl}, sc.debug_step_out},
        //        // TODO: This does not work
        named(KeyEvent{Key::Home}, {sc.home}, "home"),
        named(KeyEvent{Key::End}, {sc.end}, "end"),
        named(KeyEvent{Key::PageUp}, {sc.page_up}, "page_up"),
        named(KeyEvent{Key::PageDown}, {sc.page_down}, "page_down"),
        {KeyEvent{Key::KeyCombination, 'T', Modifiers::Ctrl},
         {[](Ptr env) { env->mainWindow().showConsole(); }}},
        {KeyEvent{Key::KeyCombination, 'E', Modifiers::Ctrl},
         {[](Ptr env) { env->mainWindow().switchEditor(); }}},
        named(KeyEvent{Key::KeyCombination, '7', Modifiers::Ctrl},
              sc.toggle_comment,
              "toggle_comment"),
        //        {KeyEvent{Key::KeyCombination, 'O', Modifiers::Ctrl},
        //         {[](Ptr env) { env->mainWindow().showOpen(); }}},
        //         {KeyEvent{ey::KeyCombination, 'O', Modifiers::Ctrl |
        //         Modifiers::Alt},
        //          sc.f("browse_files")},
        named(KeyEvent{Key::KeyCombination, 'O', Modifiers::Ctrl},
              sc.f("show_locator"),
              "show_locator"),
        named(KeyEvent{Key::KeyCombination, 'S', Modifiers::Ctrl},
              sc.combine(sc.format, sc.save),
              "save"),
        named(KeyEvent{Key::KeyCombination, 'C', Modifiers::Ctrl},
              sc.f("copy"),
              "copy"),
        named(KeyEvent{Key::KeyCombination, 'X', Modifiers::Ctrl},
              sc.f("cut"),
              "cut"),
        named(KeyEvent{Key::KeyCombination, 'Z', Modifiers::Ctrl},
              sc.undo,
              "undo"),
        named(KeyEvent{Key::KeyCombination, 'Y', Modifiers::Ctrl},
              sc.redo,
              "redo"),
        named(KeyEvent{Key::KeyCombination, 'B', Modifiers::Ctrl},
              sc.combine(sc.save, sc.build),
              "build"),
        named(KeyEvent{Key::KeyCombination, 'W', Modifiers::Ctrl},
              sc.close_buffer,
              "close_buffer"),
        named(KeyEvent{Key::KeyCombination, 'K', Modifiers::Ctrl},
              {[](Ptr env) { env->mainWindow().showLocator(); }},
              "show_locator"),
        {KeyEvent{Key::KeyCombination, ' ', Modifiers::Ctrl},
         {[](Ptr env) { env->mainWindow().autoComplete(); }}},
        named(KeyEvent{Key::KeyCombination, 'R', Modifiers::Ctrl},
              sc.f("rename_symbol"),
              "rename_symbol"),
        named(KeyEvent{Key::KeyCombination, 'A', Modifiers::Ctrl},
              sc.select_all,
              "select_all"),
        named(KeyEvent{Key::KeyCombination, 'P', Modifiers::Ctrl},
              sc.f("command_palette"),
              "command_palette"),
        named(KeyEvent{Key::KeyCombination, 'N', Modifiers::Ctrl},
              sc.new_file,
              "new_file"),
        named(KeyEvent{Key::Left, {}, Modifiers::Alt}, sc.back, "back"),
        named(KeyEvent{Key::Right, {}, Modifiers::Alt}, sc.forward, "forward"),
        named(KeyEvent{Key::KeyCombination, 'L', Modifiers::Ctrl},
              sc.f("toggle_checkmark"),
              "toggle_checkmark"),
        named(KeyEvent{Key::KeyCombination, '\t', Modifiers::Ctrl},
              sc.f("switch_file"),
              "switch_file"),
    }};

    map.defaultAction({});

    return std::make_shared<Mode>("parent", std::move(map));
}
