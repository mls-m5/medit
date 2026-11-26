#include "mls-unit-test/unittest.h"
#include "mock/screen/mockscreen.h"
#include "views/mainwindowlayout.h"
#include "views/sidebar.h"

TEST_SUIT_BEGIN(Sidebar)

TEST_CASE("sidebar leaves editor splits and console inside the window") {
    auto layout = MainWindowLayout::calculate(100, 30, 2, true, true, 10);
    ASSERT_EQ(layout.sidebarWidth, 25u);
    ASSERT_EQ(layout.editorHeight, 20u);
    ASSERT_EQ(layout.editorX(0, 2), 25u);
    ASSERT_EQ(layout.editorColumns(0, 2), 38u);
    ASSERT_EQ(layout.editorX(1, 2), 63u);
    ASSERT_EQ(layout.editorColumns(1, 2), 37u);
    ASSERT_EQ(layout.editorHeight + layout.consoleRows + 1, 30u);
}

TEST_CASE("hidden or narrow sidebar leaves all columns for editors") {
    auto hidden = MainWindowLayout::calculate(100, 30, 2, false, false, 10);
    ASSERT_EQ(hidden.sidebarWidth, 0u);
    auto narrow = MainWindowLayout::calculate(40, 30, 2, true, false, 10);
    ASSERT_EQ(narrow.sidebarWidth, 0u);
    auto wide = MainWindowLayout::calculate(41, 30, 2, true, false, 10);
    ASSERT_EQ(wide.editorColumns(0, 2), 8u);
}

TEST_CASE("small window layouts never exceed the available dimensions") {
    for (size_t width = 0; width < 45; ++width) {
        for (size_t height = 0; height < 15; ++height) {
            auto layout =
                MainWindowLayout::calculate(width, height, 2, true, true, 10);
            ASSERT_EQ(layout.sidebarWidth + layout.editorWidth, width);
            ASSERT_EQ(layout.editorHeight + layout.consoleRows,
                      (height > 0 ? height - 1 : 0));
            ASSERT_EQ(layout.editorX(1, 2) + layout.editorColumns(1, 2), width);
        }
    }
}

TEST_CASE("tree uses only registered paths and sorts directories first") {
    SideBar sidebar{nullptr};
    sidebar.height(6);
    sidebar.files("/tmp/tree",
                  {"/tmp/tree/z.cpp",
                   "/tmp/tree/a.cpp",
                   "/tmp/tree/folder/b.cpp",
                   "/tmp/outside.cpp"});
    ASSERT_EQ(sidebar.entries().size(), 4u);
    ASSERT_EQ(sidebar.entries()[0].path, SideBar::Path{"/tmp/tree"});
    ASSERT_EQ(sidebar.entries()[1].path, SideBar::Path{"/tmp/tree/folder"});
    ASSERT_EQ(sidebar.entries()[2].path, SideBar::Path{"/tmp/tree/a.cpp"});
    ASSERT_EQ(sidebar.entries()[3].path, SideBar::Path{"/tmp/tree/z.cpp"});
    sidebar.handleKey(KeyEvent{Key::Down});
    sidebar.handleKey(KeyEvent{Key::Right});
    ASSERT_EQ(sidebar.entries()[2].path,
              SideBar::Path{"/tmp/tree/folder/b.cpp"});
    sidebar.handleKey(KeyEvent{Key::Left});
    ASSERT_EQ(sidebar.entries().size(), 4u);
}

TEST_CASE("Enter opens the real path and snapshot preserves selection") {
    SideBar sidebar{nullptr};
    sidebar.height(4);
    const SideBar::Path root{"/tmp/tree"};
    const SideBar::Path file = root / "nested" / "file.cpp";
    SideBar::Path opened;
    sidebar.onOpen([&](const auto &path) { opened = path; });
    sidebar.files(root, {file});
    sidebar.handleKey(KeyEvent{Key::Down});
    sidebar.handleKey(KeyEvent{Key::Right});
    sidebar.handleKey(KeyEvent{Key::Down});
    sidebar.files(root, {file});
    ASSERT_EQ(sidebar.entries()[sidebar.current()].path, file);
    sidebar.handleKey(KeyEvent{Key::Text, '\n'});
    ASSERT_EQ(opened, file);
    sidebar.files(root, {});
    ASSERT_EQ(sidebar.entries()[sidebar.current()].path, root);
}

TEST_CASE("tree rendering clips and escapes multicolumn filenames") {
    SideBar sidebar{nullptr};
    sidebar.width(10);
    sidebar.height(3);
    sidebar.files("/tmp/tree", {"/tmp/tree/\xe7\x95\x8c.cpp"});
    MockScreen screen;
    bool escaped = false;
    screen.mock_draw_3.expectNum(6);
    screen.mock_draw_3.expectArgs([&](auto x, auto y, FStringView text) {
        const auto line = static_cast<std::string>(text);
        escaped |= line.find("\\xe7") != std::string::npos;
        return x + text.size() <= 10 && y < 3;
    });
    sidebar.draw(screen);
    ASSERT_EQ(escaped, true);
}

TEST_SUIT_END
