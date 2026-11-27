#include "core/coreenvironment.h"
#include "core/jobqueue.h"
#include "core/meditlog.h"
#include "core/timer.h"
#include "mls-unit-test/unittest.h"
#include "mock/screen/mockscreen.h"
#include "views/mainwindow.h"
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <stdexcept>

namespace sidebar_focus_test {

// CoreEnvironment uses per-process FIFOs and project history under HOME.
// Isolate these tests from the user's editor configuration.
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

    explicit WindowFixture(size_t width = 100) {
        window.resize(width, 30);
        // Keep assertion diagnostics out of the editor's captured log stream.
        unsubscribeToLog();
    }

    void key(KeyEvent event) {
        window._env->key(event);
        window.keyPress(window._env);
    }

    void typePalette(std::string_view name) {
        key({Key::KeyCombination, 'P', Modifiers::Ctrl});
        for (char c : name) {
            key({Key::Text, c});
        }
    }

    bool sidebarFocused() const {
        return window._inputFocus == &window._sidebar;
    }
};

} // namespace sidebar_focus_test

TEST_SUIT_BEGIN(SidebarFocus)

TEST_CASE("F6 opens a hidden sidebar and toggles focus without hiding it") {
    sidebar_focus_test::WindowFixture f;
    EXPECT_FALSE(f.window._sidebar.visible());

    f.key({Key::F6});
    EXPECT_TRUE(f.window._sidebar.visible());
    EXPECT_TRUE(f.sidebarFocused());

    f.key({Key::F6});
    EXPECT_TRUE(f.window._sidebar.visible());
    EXPECT_FALSE(f.sidebarFocused());

    f.key({Key::F6});
    EXPECT_TRUE(f.sidebarFocused());
}

TEST_CASE("F3 still toggles visibility and F6 does not focus a hidden panel") {
    sidebar_focus_test::WindowFixture f;
    f.key({Key::F3});
    EXPECT_TRUE(f.sidebarFocused());
    f.key({Key::F3});
    EXPECT_FALSE(f.window._sidebar.visible());
    EXPECT_FALSE(f.sidebarFocused());

    sidebar_focus_test::WindowFixture narrow{30};
    narrow.key({Key::F6});
    EXPECT_FALSE(narrow.window._sidebar.visible());
    EXPECT_FALSE(narrow.sidebarFocused());
}

TEST_CASE("Escape returns from the sidebar without hiding it") {
    sidebar_focus_test::WindowFixture f;
    f.key({Key::F6});
    f.key({Key::Escape});
    EXPECT_TRUE(f.window._sidebar.visible());
    EXPECT_FALSE(f.sidebarFocused());
}

TEST_CASE("palette commands can move focus to and from the sidebar") {
    sidebar_focus_test::WindowFixture f;
    f.typePalette("focus_sidebar");
    f.key({Key::Text, '\n'});
    EXPECT_TRUE(f.sidebarFocused());

    f.typePalette("focus_editor");
    f.key({Key::Text, '\n'});
    EXPECT_FALSE(f.sidebarFocused());
    EXPECT_TRUE(f.window._sidebar.visible());
}

TEST_CASE("cancelling the palette restores the focus it was opened from") {
    sidebar_focus_test::WindowFixture f;
    f.key({Key::F6});
    f.key({Key::KeyCombination, 'P', Modifiers::Ctrl});
    f.key({Key::Escape});
    EXPECT_TRUE(f.sidebarFocused());

    f.key({Key::F6});
    EXPECT_FALSE(f.sidebarFocused());
    f.key({Key::KeyCombination, 'P', Modifiers::Ctrl});
    f.key({Key::Escape});
    EXPECT_FALSE(f.sidebarFocused());
}

TEST_SUIT_END
