#include "keys/event.h"
#include "mls-unit-test/unittest.h"
#include "modes/imode.h"
#include "modes/parentmode.h"
#include "script/standardcommands.h"

TEST_SUIT_BEGIN(Shortcut)

TEST_CASE("key events describe themselves as shortcuts") {
    ASSERT_EQ((KeyEvent{Key::F4}.description()), "F4");
    ASSERT_EQ((KeyEvent{Key::F11}.description()), "F11");
    ASSERT_EQ((KeyEvent{Key::PageUp}.description()), "PageUp");
    ASSERT_EQ(
        (KeyEvent{Key::KeyCombination, 'O', Modifiers::Ctrl}.description()),
        "Ctrl+O");
    ASSERT_EQ(
        (KeyEvent{Key::KeyCombination, '\t', Modifiers::Ctrl}.description()),
        "Ctrl+Tab");
    ASSERT_EQ(
        (KeyEvent{Key::KeyCombination, ' ', Modifiers::Ctrl}.description()),
        "Ctrl+Space");
    ASSERT_EQ((KeyEvent{Key::Left, {}, Modifiers::Alt}.description()),
              "Alt+Left");
    ASSERT_EQ((KeyEvent{Key::Unknown}.description()), "");
}

TEST_CASE("shortcuts are collected per command without duplicates") {
    auto &sc = StandardCommands::get();
    const auto name = std::string{"shortcut_test_command"};
    ASSERT_EQ(sc.shortcutDescription(name), "");
    sc.shortcut(name, KeyEvent{Key::F7});
    sc.shortcut(name, KeyEvent{Key::KeyCombination, 'Q', Modifiers::Ctrl});
    sc.shortcut(name, KeyEvent{Key::F7});
    ASSERT_EQ(sc.shortcutDescription(name), "F7, Ctrl+Q");
    sc.shortcuts.erase(name);
}

TEST_CASE("the parent mode registers its shortcuts and keeps its bindings") {
    auto mode = createParentMode();
    auto &sc = StandardCommands::get();
    ASSERT_EQ(sc.shortcutDescription("command_palette"), "Ctrl+P");
    ASSERT_EQ(sc.shortcutDescription("show_locator"), "Ctrl+O, Ctrl+K");
    ASSERT_EQ(sc.shortcutDescription("switch_header"), "F4");
    ASSERT_EQ(sc.shortcutDescription("save"), "Ctrl+S");

    // Creating the mode again (one per editor) adds no duplicates.
    auto again = createParentMode();
    ASSERT_EQ(sc.shortcutDescription("show_locator"), "Ctrl+O, Ctrl+K");

    ASSERT_EQ(std::string{mode->name()}, "parent");
}

TEST_SUIT_END
