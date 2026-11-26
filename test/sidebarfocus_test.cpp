#include "core/coreenvironment.h"
#include "core/jobqueue.h"
#include "core/meditlog.h"
#include "core/timer.h"
#include "mls-unit-test/unittest.h"
#include "mock/screen/mockscreen.h"
#include "script/interaction.h"
#include "script/saveinteraction.h"
#include "views/commandpalette.h"
#include "views/locator.h"
#include "views/mainwindow.h"
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <stdexcept>

namespace sidebar_focus_test {

// CoreEnvironment uses per-process FIFOs and project history under HOME.
// Isolate these integration tests from the user's editor configuration.
struct TemporaryHome {
    std::optional<std::string> previous;
    std::filesystem::path path;

    TemporaryHome() {
        if (const auto *home = std::getenv("HOME")) {
            previous = home;
        }
        auto pattern = (std::filesystem::temp_directory_path() /
                        "medit-sidebar-test-XXXXXX")
                           .string();
        const auto *created = mkdtemp(pattern.data());
        if (!created) {
            throw std::runtime_error("Cannot create temporary test home");
        }
        path = created;
        setenv("HOME", path.c_str(), 1);
    }

    ~TemporaryHome() {
        if (previous) {
            setenv("HOME", previous->c_str(), 1);
        }
        else {
            unsetenv("HOME");
        }
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
};

struct WindowFixture {
    TemporaryHome home;
    JobQueue jobs;
    JobQueue gui;
    Timer timer;
    ThreadContext context{jobs, gui, timer};
    CoreEnvironment core{context,
                         std::filesystem::absolute("test/res/.medit.json")};
    MockScreen screen;

    IScreen &prepareScreen() {
        screen.mock_palette_1.expectNum(1);
        return screen;
    }

    MainWindow window{core, prepareScreen(), context};

    WindowFixture() {
        window.resize(100, 30);
        // Keep assertion diagnostics out of the editor's captured log stream.
        unsubscribeToLog();
    }

    void key(KeyEvent event) {
        window._env->key(event);
        window.keyPress(window._env);
    }

    void paletteCommand(std::string_view name) {
        key({Key::KeyCombination, 'P', Modifiers::Ctrl});
        for (char c : name) {
            key({Key::Text, c});
        }
        key({Key::Text, '\n'});
    }
};

} // namespace sidebar_focus_test

TEST_SUIT_BEGIN(SidebarFocus)

TEST_CASE("F3 opens and focuses the sidebar, then returns to the editor") {
    sidebar_focus_test::WindowFixture f;
    f.window.splitEditor();
    f.window.resize();
    auto *editor = f.window._editors[1].get();
    f.window.mouseDown(editor->x() + 4, 0);
    f.key(Key::F3);
    EXPECT_TRUE(f.window._sidebar.visible());
    EXPECT_TRUE(f.window._sidebar.focused());
    EXPECT_TRUE(f.window._inputFocus == &f.window._sidebar);
    EXPECT_TRUE(f.window.currentEditor() == editor);
    f.key(Key::F3);
    EXPECT_FALSE(f.window._sidebar.visible());
    EXPECT_TRUE(f.window._inputFocus == editor);
}

TEST_CASE("F3 does not focus a sidebar hidden by a narrow window") {
    sidebar_focus_test::WindowFixture f;
    f.window.resize(20, 6);
    auto *editor = f.window.currentEditor();
    f.key(Key::F3);
    EXPECT_FALSE(f.window._sidebar.visible());
    EXPECT_TRUE(f.window._inputFocus == editor);
}

TEST_CASE("F6 opens the sidebar and returns to the last text editor") {
    sidebar_focus_test::WindowFixture f;
    auto *editor = f.window.currentEditor();
    EXPECT_FALSE(f.window._sidebar.visible());
    f.key(Key::F6);
    EXPECT_TRUE(f.window._sidebar.visible());
    EXPECT_TRUE(f.window._sidebar.focused());
    EXPECT_TRUE(f.window._inputFocus == &f.window._sidebar);
    EXPECT_TRUE(f.window.currentEditor() == editor);
    f.key(Key::F6);
    EXPECT_TRUE(f.window._inputFocus == editor);
    EXPECT_FALSE(f.window._sidebar.focused());
    EXPECT_TRUE(f.window._sidebar.visible());
}

TEST_CASE("Escape leaves the sidebar visible and does not close the console") {
    sidebar_focus_test::WindowFixture f;
    f.window.showConsole();
    f.window.focusSidebar();
    f.key(Key::Escape);
    EXPECT_TRUE(f.window._inputFocus == f.window._editors.front().get());
    EXPECT_TRUE(f.window._sidebar.visible());
    EXPECT_TRUE(f.window._env->showConsole());
}

TEST_CASE("hiding a focused sidebar returns focus to the editor") {
    sidebar_focus_test::WindowFixture f;
    f.window.focusSidebar();
    f.key(Key::F3);
    EXPECT_FALSE(f.window._sidebar.visible());
    EXPECT_FALSE(f.window._sidebar.focused());
    EXPECT_TRUE(f.window._inputFocus == f.window._editors.front().get());
}

TEST_CASE("resize cannot leave keyboard focus in a hidden sidebar") {
    sidebar_focus_test::WindowFixture f;
    f.window.focusSidebar();
    f.window.resize(20, 6);
    EXPECT_FALSE(f.window._sidebar.visible());
    EXPECT_TRUE(f.window._inputFocus == f.window._editors.front().get());
    f.window.focusSidebar();
    EXPECT_FALSE(f.window._sidebar.focused());
    f.window.resize(100, 30);
    EXPECT_TRUE(f.window._sidebar.visible());
    EXPECT_FALSE(f.window._sidebar.focused());
}

TEST_CASE("mouse focus respects sidebar and separator boundaries") {
    sidebar_focus_test::WindowFixture f;
    f.window.toggleSidebar();
    EXPECT_TRUE(f.window.mouseDown(0, 0));
    EXPECT_TRUE(f.window._sidebar.focused());
    f.window.focusEditor();
    EXPECT_TRUE(f.window.mouseDown(23, 28));
    EXPECT_TRUE(f.window._sidebar.focused());
    f.window.focusEditor();
    EXPECT_TRUE(f.window.mouseDown(24, 0)); // Separator: consume, do not focus.
    EXPECT_FALSE(f.window._sidebar.focused());
    EXPECT_FALSE(f.window.mouseDown(0, 29)); // Status bar.
    EXPECT_FALSE(f.window.mouseDown(-1, 0));
    EXPECT_FALSE(f.window.mouseDown(0, -1));
    f.window.focusSidebar();
    EXPECT_TRUE(f.window.mouseDown(25, 0)); // First editor column.
    EXPECT_FALSE(f.window._sidebar.focused());
    EXPECT_TRUE(f.window._inputFocus == f.window._editors.front().get());
}

TEST_CASE("sidebar focus preserves the selected editor split") {
    sidebar_focus_test::WindowFixture f;
    f.window.splitEditor();
    f.window.resize();
    auto *second = f.window._editors[1].get();
    EXPECT_TRUE(f.window.mouseDown(second->x() + 4, 0));
    f.window.focusSidebar();
    EXPECT_TRUE(f.window.currentEditor() == second);
    f.key(Key::F6);
    EXPECT_TRUE(f.window._inputFocus == second);
}

TEST_CASE("tool input and paste cannot modify the background text editor") {
    sidebar_focus_test::WindowFixture f;
    auto *editor = f.window.currentEditor();
    editor->buffer().text(std::string{"unchanged"});
    auto *buffer = &editor->buffer();
    f.window.focusSidebar();
    for (char c : std::string{"ixd\n"}) {
        f.key({Key::Text, c});
    }
    for (char c : std::string{"ASWXZNYOKE"}) {
        f.key({Key::KeyCombination, c, Modifiers::Ctrl});
    }
    f.window.paste("must not be inserted");
    f.window.copy(true);
    f.window.copy(false);
    f.window.autoComplete();
    f.window.format();
    EXPECT_TRUE(&editor->buffer() == buffer);
    ASSERT_EQ(editor->buffer().text(), "unchanged");
    EXPECT_TRUE(f.window._sidebar.focused());
}

TEST_CASE("manual save asks before overwriting an external change") {
    sidebar_focus_test::WindowFixture f;
    const auto path = f.home.path / "conflict.cpp";
    std::ofstream{path} << "original\n";
    const auto originalTime = std::filesystem::last_write_time(path);
    auto buffer = f.core.files().open(path);
    f.window.currentEditor()->buffer(buffer);
    buffer->text(std::string{"editor\n"});
    std::ofstream{path} << "external\n";
    std::filesystem::last_write_time(path,
                                     originalTime + std::chrono::seconds{2});
    bool called = false;
    bool saved = false;
    saveInteraction(f.window._env, [&](bool result) {
        called = true;
        saved = result;
    });
    EXPECT_TRUE(f.window.interactions().isOperationBuffer(
        &f.window.currentEditor()->buffer()));
    EXPECT_FALSE(called);
    std::ifstream before{path};
    std::string text;
    std::getline(before, text);
    ASSERT_EQ(text, "external");
    f.key({Key::Text, '\n'}); // Default selection is Yes.
    f.gui.work(false);
    EXPECT_TRUE(called);
    EXPECT_TRUE(saved);
    std::ifstream after{path};
    std::getline(after, text);
    ASSERT_EQ(text, "editor");
}

TEST_CASE("sidebar Enter cannot accept a background interaction") {
    sidebar_focus_test::WindowFixture f;
    Interaction interaction;
    interaction.text = "operation";
    f.window.interactions().newInteraction(interaction, {});
    auto *operation = &f.window.currentEditor()->buffer();
    f.window.focusSidebar();
    f.key({Key::Text, '\n'});
    EXPECT_TRUE(f.window.interactions().isOperationBuffer(operation));
}

TEST_CASE("command palette can focus the sidebar without overriding focus") {
    sidebar_focus_test::WindowFixture f;
    f.paletteCommand("focus sidebar");
    EXPECT_TRUE(f.window._sidebar.focused());
    EXPECT_TRUE(f.window._inputFocus == &f.window._sidebar);
    EXPECT_FALSE(f.window._commandPalette->visible());
    f.paletteCommand("focus editor");
    EXPECT_FALSE(f.window._sidebar.focused());
    EXPECT_TRUE(f.window._inputFocus == f.window._editors.front().get());
}

TEST_CASE("cancelling the command palette returns to the sidebar") {
    sidebar_focus_test::WindowFixture f;
    f.window.focusSidebar();
    f.key({Key::KeyCombination, 'P', Modifiers::Ctrl});
    EXPECT_TRUE(f.window._commandPalette->visible());
    f.key(Key::Escape);
    EXPECT_TRUE(f.window._sidebar.focused());
    EXPECT_FALSE(f.window._commandPalette->visible());
}

TEST_CASE("Ctrl+T opens the console from both editor and sidebar") {
    sidebar_focus_test::WindowFixture f;
    for (bool sidebar : {false, true}) {
        if (sidebar) {
            f.window.focusSidebar();
        }
        f.key({Key::KeyCombination, 'T', Modifiers::Ctrl});
        EXPECT_TRUE(f.window._env->showConsole());
        EXPECT_TRUE(f.window._inputFocus == f.window._console.get());
        // In insert mode, Escape first returns to normal mode, then closes.
        f.key({Key::Text, 'i'});
        f.key(Key::Escape);
        EXPECT_TRUE(f.window._env->showConsole());
        f.key(Key::Escape);
        EXPECT_FALSE(f.window._env->showConsole());
    }
}

TEST_CASE(
    "focus commands dismiss editor popups instead of leaving stale targets") {
    sidebar_focus_test::WindowFixture f;
    f.window.showLocator();
    f.window.focusSidebar();
    EXPECT_FALSE(f.window._locator->visible());
    EXPECT_TRUE(f.window.currentEditor() == f.window._editors.front().get());
    f.window.showCommandPalette();
    f.window.focusEditor();
    EXPECT_FALSE(f.window._commandPalette->visible());
    EXPECT_TRUE(f.window.currentEditor() == f.window._editors.front().get());
}

TEST_CASE("unmapped window keys reach the editor's existing mode") {
    sidebar_focus_test::WindowFixture f;
    EXPECT_FALSE(f.window._windowKeyMap.defaultAction());
    EXPECT_FALSE(f.window._windowKeyMap.find({Key::Text, 'a'}));
    EXPECT_FALSE(f.window._windowKeyMap.find(
        {Key::KeyCombination, 'O', Modifiers::Ctrl}));
    f.key({Key::Text, 'i'}); // Enter insert mode through the editor's Vim map.
    f.key({Key::Text, 'a'});
    ASSERT_EQ(f.window.currentEditor()->buffer().text(), "a");
    f.key({Key::KeyCombination, 'O', Modifiers::Ctrl});
    EXPECT_TRUE(f.window._locator->visible());
    f.key(Key::Escape);
    EXPECT_FALSE(f.window._locator->visible());
}

TEST_CASE("background errors appear in the console without taking focus") {
    sidebar_focus_test::WindowFixture f;
    auto *editor = f.window.currentEditor();
    for (int i = 0; i < 3; ++i)
        f.core.printToAllLogs(LogType::Error, "language server error");
    f.gui.work(false);
    EXPECT_TRUE(f.window._env->showConsole());
    EXPECT_TRUE(f.window._inputFocus == editor);
    EXPECT_TRUE(f.window._console->buffer().text().find(
                    "language server error") != std::string::npos);
    f.window.focusSidebar();
    f.core.printToAllLogs(LogType::Error, "another error");
    f.gui.work(false);
    EXPECT_TRUE(f.window._inputFocus == &f.window._sidebar);
    f.window.showConsole();
    EXPECT_TRUE(f.window._inputFocus == f.window._console.get());
}

TEST_CASE("window shortcuts work while the console has focus") {
    sidebar_focus_test::WindowFixture f;
    f.window.showConsole();
    f.key({Key::KeyCombination, 'P', Modifiers::Ctrl});
    EXPECT_TRUE(f.window._commandPalette->visible());
    f.key(Key::Escape);
    f.window.showConsole();
    f.key(Key::F6);
    EXPECT_TRUE(f.window._sidebar.focused());
    f.window.showConsole();
    f.key(Key::F3);
    EXPECT_FALSE(f.window._sidebar.visible());
    EXPECT_TRUE(f.window._inputFocus == f.window._console.get());
}

TEST_CASE("sidebar Enter opens a registered file in the last text split") {
    sidebar_focus_test::WindowFixture f;
    f.window.splitEditor();
    f.window.resize();
    auto *editor = f.window._editors[1].get();
    f.window.mouseDown(editor->x() + 4, 0);
    f.window.showConsole();
    f.window.focusSidebar();
    const auto path =
        std::filesystem::absolute("test/res/annotation_test1.cpp");
    EXPECT_TRUE(f.window._sidebar.tree().reveal(path));
    f.screen.mock_title_1.expectNum(1);
    f.key({Key::Text, '\n'});
    EXPECT_TRUE(f.window._inputFocus == editor);
    ASSERT_EQ(editor->file()->path(), path);
    EXPECT_TRUE(f.window._sidebar.visible());
    EXPECT_TRUE(f.window._env->showConsole());
}

TEST_CASE(
    "clicking a file opens it without restoring sidebar focus afterward") {
    sidebar_focus_test::WindowFixture f;
    f.window.focusSidebar();
    auto &tree = f.window._sidebar.tree();
    const auto path = std::filesystem::absolute("test/res/complete_test1.cpp");
    EXPECT_TRUE(tree.reveal(path));
    const auto row = tree.y() + tree.current() - tree.yScroll();
    f.screen.mock_title_1.expectNum(1);
    EXPECT_TRUE(f.window.mouseDown(tree.x() + 3, row));
    EXPECT_TRUE(f.window._inputFocus == f.window._editors.front().get());
    ASSERT_EQ(f.window.currentEditor()->file()->path(), path);
}

TEST_CASE("following active text files reveals without stealing focus or "
          "resetting navigation") {
    sidebar_focus_test::WindowFixture f;
    f.window.focusSidebar();
    f.window.focusEditor();
    f.screen.mock_title_1.expectNum(3);
    f.screen.mock_cursorStyle_1.expectNum(4);
    f.screen.mock_draw_3.nice();
    f.screen.mock_draw_3.expectArgs([&](auto x, auto y, FStringView text) {
        return x + text.size() <= f.window.width() && y < f.window.height();
    });
    const auto first =
        std::filesystem::absolute("test/res/annotation_test1.cpp");
    const auto second =
        std::filesystem::absolute("test/res/complete_test1.cpp");
    EXPECT_TRUE(f.window.open(first));
    auto *focus = f.window._inputFocus;
    f.window.draw(f.screen);
    auto &tree = f.window._sidebar.tree();
    ASSERT_EQ(tree.entries()[tree.current()].path, first);
    EXPECT_TRUE(f.window._inputFocus == focus);
    tree.current(0);
    f.window.draw(f.screen);
    ASSERT_EQ(tree.current(), 0u);
    f.paletteCommand("toggle follow active file");
    EXPECT_FALSE(f.window._sidebar.followActiveFile());
    EXPECT_TRUE(f.window.open(second));
    f.window.draw(f.screen);
    ASSERT_EQ(tree.current(), 0u);
    f.paletteCommand("toggle follow active file");
    EXPECT_TRUE(f.window._sidebar.followActiveFile());
    ASSERT_EQ(tree.entries()[tree.current()].path, second);
    EXPECT_TRUE(f.window._inputFocus == focus);
    f.window.focusSidebar();
    EXPECT_TRUE(f.window.open(first));
    f.window.draw(f.screen);
    ASSERT_EQ(tree.entries()[tree.current()].path, first);
    EXPECT_TRUE(f.window._inputFocus == &f.window._sidebar);
}

TEST_SUIT_END
