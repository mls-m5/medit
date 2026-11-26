#include "mls-unit-test/unittest.h"
#include "mock/screen/mockscreen.h"
#include "syntax/palette.h"
#include "views/projecttreeview.h"

namespace project_tree_test {
using Path = std::filesystem::path;
const Path root = "/project";
std::vector<Path> files() {
    return {root / "README.md",
            root / "src/views/editor.cpp",
            root / "src/main.cpp",
            root / "data/theme.json",
            root / "src/main.cpp",
            root / "src/../README.md",
            "/outside/settings.json"};
}
Path selected(const ProjectTreeView &tree) {
    return tree.entries().empty() ? Path{}
                                  : tree.entries()[tree.current()].path;
}
} // namespace project_tree_test

TEST_SUIT_BEGIN(ProjectTree)

TEST_CASE("registered paths create implicit directories sorted before files") {
    ProjectTreeView tree{nullptr};
    tree.files(project_tree_test::root, project_tree_test::files());
    ASSERT_EQ(tree.entries().size(), 4u);
    ASSERT_EQ(tree.entries()[0].path, project_tree_test::root);
    ASSERT_EQ(tree.entries()[1].path, project_tree_test::root / "data");
    ASSERT_EQ(tree.entries()[2].path, project_tree_test::root / "src");
    ASSERT_EQ(tree.entries()[3].path, project_tree_test::root / "README.md");
    EXPECT_TRUE(tree.entries()[1].directory);
    EXPECT_FALSE(tree.entries()[3].directory);
    ASSERT_EQ(tree.entries()[2].depth, 1u);
    tree.reveal("/project/src/views/editor.cpp");
    ASSERT_EQ(tree.entries().size(), 7u);
    ASSERT_EQ(tree.entries()[3].path, project_tree_test::root / "src/views");
    ASSERT_EQ(tree.entries()[4].depth, 3u);
}

TEST_CASE("arrows and hjkl expand collapse and select parents") {
    ProjectTreeView tree{nullptr};
    tree.files("/project", project_tree_test::files());
    tree.handleKey(Key::Down);
    ASSERT_EQ(project_tree_test::selected(tree),
              std::filesystem::path{"/project/data"});
    tree.handleKey({Key::Text, 'l'});
    EXPECT_TRUE(tree.expanded("/project/data"));
    tree.handleKey(Key::Right);
    ASSERT_EQ(project_tree_test::selected(tree),
              std::filesystem::path{"/project/data/theme.json"});
    tree.handleKey({Key::Text, 'h'});
    ASSERT_EQ(project_tree_test::selected(tree),
              std::filesystem::path{"/project/data"});
    tree.handleKey(Key::Left);
    EXPECT_FALSE(tree.expanded("/project/data"));
    tree.handleKey({Key::Text, 'k'});
    ASSERT_EQ(tree.current(), 0u);
    tree.handleKey({Key::Text, 'j'});
    ASSERT_EQ(tree.current(), 1u);
}

TEST_CASE(
    "Enter toggles directories and opens only a selected registered file") {
    ProjectTreeView tree{nullptr};
    tree.files("/project", project_tree_test::files());
    std::filesystem::path opened;
    size_t calls = 0;
    tree.onOpen([&](const auto &path) {
        opened = path;
        ++calls;
    });
    tree.handleKey({Key::Text, '\n'});
    ASSERT_EQ(tree.entries().size(), 1u);
    ASSERT_EQ(calls, 0u);
    tree.handleKey({Key::Text, '\n'});
    tree.reveal("/project/src/views/editor.cpp");
    tree.handleKey({Key::Text, '\n'});
    ASSERT_EQ(calls, 1u);
    ASSERT_EQ(opened, std::filesystem::path{"/project/src/views/editor.cpp"});
}

TEST_CASE("unmatched modified and key-up events cannot activate a file") {
    ProjectTreeView tree{nullptr};
    tree.files("/project", project_tree_test::files());
    tree.reveal("/project/README.md");
    size_t calls = 0;
    tree.onOpen([&](const auto &) { ++calls; });
    EXPECT_FALSE(tree.handleKey({Key::Text, 'x'}));
    EXPECT_FALSE(tree.handleKey({Key::Text, '\n', Modifiers::Ctrl}));
    EXPECT_FALSE(tree.handleKey({Key::Text, '\n', Modifiers::None, false}));
    ASSERT_EQ(calls, 0u);
}

TEST_CASE("refresh preserves expanded directories selection and scroll") {
    ProjectTreeView tree{nullptr};
    tree.height(3);
    auto files = project_tree_test::files();
    tree.files("/project", files);
    tree.reveal("/project/src/views/editor.cpp");
    const auto scroll = tree.yScroll();
    tree.files("/project", files);
    ASSERT_EQ(project_tree_test::selected(tree),
              std::filesystem::path{"/project/src/views/editor.cpp"});
    EXPECT_TRUE(tree.expanded("/project/src"));
    EXPECT_TRUE(tree.expanded("/project/src/views"));
    ASSERT_EQ(tree.yScroll(), scroll);
    files.push_back("/project/src/views/sidebar.cpp");
    tree.files("/project", files);
    ASSERT_EQ(project_tree_test::selected(tree),
              std::filesystem::path{"/project/src/views/editor.cpp"});
}

TEST_CASE(
    "removing selection falls back to a visible parent and prunes expansion") {
    ProjectTreeView tree{nullptr};
    tree.files("/project", project_tree_test::files());
    tree.reveal("/project/src/views/editor.cpp");
    tree.files("/project", {"/project/src/main.cpp"});
    ASSERT_EQ(project_tree_test::selected(tree),
              std::filesystem::path{"/project/src"});
    EXPECT_FALSE(tree.expanded("/project/src/views"));
    tree.files("/other", {"/other/readme.md"});
    ASSERT_EQ(project_tree_test::selected(tree),
              std::filesystem::path{"/other"});
    EXPECT_FALSE(tree.expanded("/project/src"));
    EXPECT_TRUE(tree.expanded("/other"));
}

TEST_CASE(
    "reveal expands ancestors but does not open files or add unknown paths") {
    ProjectTreeView tree{nullptr};
    tree.files("/project", project_tree_test::files());
    size_t calls = 0;
    tree.onOpen([&](const auto &) { ++calls; });
    EXPECT_TRUE(tree.reveal("/project/src/./views/editor.cpp"));
    ASSERT_EQ(calls, 0u);
    const auto count = tree.entries().size();
    const auto selected = tree.current();
    EXPECT_FALSE(tree.reveal("/project/unknown/new.cpp"));
    EXPECT_FALSE(tree.reveal("/elsewhere/file.cpp"));
    ASSERT_EQ(tree.entries().size(), count);
    ASSERT_EQ(tree.current(), selected);
}

TEST_CASE("scrolling and resize keep the cursor on a visible selected row") {
    ProjectTreeView tree{nullptr};
    tree.x(2);
    tree.y(4);
    tree.width(20);
    tree.height(2);
    std::vector<std::filesystem::path> files;
    for (size_t i = 0; i < 20; ++i)
        files.push_back("/project/file" + std::to_string(i) + ".cpp");
    tree.files("/project", files);
    tree.handleKey(Key::End);
    ASSERT_EQ(tree.current(), 20u);
    EXPECT_TRUE(tree.yScroll() <= tree.current());
    EXPECT_TRUE(tree.current() < tree.yScroll() + tree.height());
    MockScreen screen;
    screen.mock_cursor_2.expectNum(2);
    screen.mock_cursor_2.expectArgs([&](auto x, auto y) {
        return x == 2 && y >= 4 && y < 4 + tree.height();
    });
    tree.updateCursor(screen);
    tree.height(1);
    tree.updateCursor(screen);
    tree.height(0);
    tree.updateCursor(screen);
    tree.handleKey(Key::PageUp);
    tree.height(30);
    ASSERT_EQ(tree.yScroll(), 0u);
    tree.handleKey(Key::Home);
    ASSERT_EQ(tree.current(), 0u);
}

TEST_CASE(
    "mouse rows use the scroll offset and exclude right and bottom edges") {
    ProjectTreeView tree{nullptr};
    tree.files("/project", project_tree_test::files());
    tree.x(3);
    tree.y(5);
    tree.width(20);
    tree.height(2);
    tree.handleKey(Key::End);
    std::filesystem::path opened;
    tree.onOpen([&](const auto &path) { opened = path; });
    EXPECT_TRUE(tree.mouseDown(22, 6));
    ASSERT_EQ(opened, std::filesystem::path{"/project/README.md"});
    EXPECT_FALSE(tree.mouseDown(23, 6));
    EXPECT_FALSE(tree.mouseDown(3, 7));
    EXPECT_FALSE(tree.mouseDown(-1, 5));
    tree.visible(false);
    EXPECT_FALSE(tree.mouseDown(3, 5));
}

TEST_CASE(
    "UTF-8 markers distinguish selection directory expansion and open files") {
    ProjectTreeView tree{nullptr};
    tree.files("/project", {"/project/src/one.cpp", "/project/README.md"});
    tree.x(2);
    tree.y(3);
    tree.width(30);
    tree.height(4);
    tree.focused(true);
    tree.reveal("/project/src/one.cpp");
    tree.isOpen(
        [](const auto &path) { return path == "/project/src/one.cpp"; });
    bool selectedOpen = false, expanded = false;
    MockScreen screen;
    screen.mock_draw_3.expectNum(4);
    screen.mock_draw_3.expectArgs([&](auto x, auto y, FStringView text) {
        const auto str = std::string{text};
        if (str.find("›●") != std::string::npos)
            selectedOpen = text.front().f == Palette::currentLine;
        if (str.find("▾ src/") != std::string::npos)
            expanded = true;
        return x == 2 && y >= 3 && y < 7 && text.size() == 30;
    });
    tree.draw(screen);
    EXPECT_TRUE(selectedOpen);
    EXPECT_TRUE(expanded);
}

TEST_CASE("Git changes color only registered changed files") {
    ProjectTreeView tree{nullptr};
    tree.files("/project", {"/project/clean.cpp", "/project/changed.cpp"});
    tree.gitChanged(
        [](const auto &path) { return path == "/project/changed.cpp"; });
    tree.width(32);
    tree.height(3);
    tree.reveal("/project/changed.cpp");
    bool changed = false, clean = false, directory = false;
    MockScreen screen;
    screen.mock_draw_3.expectNum(3);
    screen.mock_draw_3.expectArgs([&](auto, auto, FStringView text) {
        const auto str = std::string{text};
        if (str.find("changed.cpp") != std::string::npos)
            changed = text.front().f == Palette::warning;
        if (str.find("clean.cpp") != std::string::npos)
            clean = text.front().f == Palette::standard;
        if (str.find("project/") != std::string::npos)
            directory = text.front().f != Palette::warning;
        return true;
    });
    tree.draw(screen);
    EXPECT_TRUE(changed);
    EXPECT_TRUE(clean);
    EXPECT_TRUE(directory);
}

TEST_CASE("filenames with double width controls and invalid bytes cannot move "
          "layout") {
    ProjectTreeView tree{nullptr};
    const auto file = std::filesystem::path{"/project/å界\t.cpp"};
    tree.files("/project",
               {file, std::filesystem::path{"/project/bad\xff.cpp"}});
    tree.reveal(file);
    tree.width(50);
    tree.height(3);
    bool escaped = false, invalidEscaped = false;
    MockScreen screen;
    screen.mock_draw_3.expectNum(3);
    screen.mock_draw_3.expectArgs([&](auto, auto, FStringView text) {
        const auto str = std::string{text};
        if (str.find("å\\u{754c}\\u{9}.cpp") != std::string::npos)
            escaped = true;
        if (str.find("bad\\xff.cpp") != std::string::npos)
            invalidEscaped = true;
        return text.size() == 50;
    });
    tree.draw(screen);
    EXPECT_TRUE(escaped);
    EXPECT_TRUE(invalidEscaped);
    std::filesystem::path opened;
    tree.onOpen([&](const auto &path) { opened = path; });
    tree.handleKey({Key::Text, '\n'});
    ASSERT_EQ(opened, file);
}

TEST_CASE("empty and tiny trees keep drawing and cursor coordinates bounded") {
    ProjectTreeView tree{nullptr};
    tree.files({}, {});
    tree.handleKey(Key::Down);
    tree.handleKey(Key::End);
    tree.handleKey(Key::Left);
    ASSERT_EQ(tree.current(), 0u);
    EXPECT_FALSE(tree.reveal("/project/file.cpp"));
    MockScreen screen;
    tree.x(2);
    tree.y(3);
    screen.mock_cursor_2.expectNum(0);
    screen.mock_draw_3.expectNum(24);
    screen.mock_draw_3.expectArgs([&](auto x, auto y, FStringView text) {
        return x == 2 && text.size() == tree.width() && y >= 3 &&
               y < 3 + tree.height();
    });
    for (size_t width = 0; width < 5; ++width) {
        for (size_t height = 0; height < 4; ++height) {
            tree.width(width);
            tree.height(height);
            tree.draw(screen);
            tree.updateCursor(screen);
        }
    }
}

TEST_CASE(
    "trailing separators and filesystem roots retain stable path identity") {
    ProjectTreeView tree{nullptr};
    tree.files("/project/./", {"/project/src/a.cpp"});
    ASSERT_EQ(tree.root(), std::filesystem::path{"/project"});
    tree.handleKey({Key::Text, '\n'});
    EXPECT_FALSE(tree.expanded("/project"));
    EXPECT_TRUE(tree.reveal("/project/src/a.cpp"));
    EXPECT_TRUE(tree.expanded("/project"));
    ASSERT_EQ(project_tree_test::selected(tree),
              std::filesystem::path{"/project/src/a.cpp"});
    tree.files("/", {"/src/a.cpp"});
    tree.handleKey({Key::Text, '\n'});
    EXPECT_TRUE(tree.reveal("/src/a.cpp"));
    EXPECT_TRUE(tree.expanded("/"));
    ASSERT_EQ(project_tree_test::selected(tree),
              std::filesystem::path{"/src/a.cpp"});
}

TEST_CASE("deep trees and UTF-8 labels stay bounded at every tiny size") {
    ProjectTreeView tree{nullptr};
    std::filesystem::path file = "/project";
    for (size_t i = 0; i < 40; ++i)
        file /= "directory";
    file /= "å界.cpp";
    tree.files("/project", {file});
    EXPECT_TRUE(tree.reveal(file));
    tree.x(2);
    tree.y(3);
    MockScreen screen;
    screen.mock_draw_3.expectNum(42);
    screen.mock_draw_3.expectArgs([&](auto x, auto y, FStringView text) {
        return x == 2 && text.size() == tree.width() && y >= 3 &&
               y < 3 + tree.height();
    });
    screen.mock_cursor_2.expectNum(21);
    screen.mock_cursor_2.expectArgs([&](auto x, auto y) {
        return x == 2 && y >= 3 && y < 3 + tree.height();
    });
    for (size_t width = 0; width < 8; ++width) {
        for (size_t height = 0; height < 4; ++height) {
            tree.width(width);
            tree.height(height);
            tree.draw(screen);
            tree.updateCursor(screen);
        }
    }
}

TEST_SUIT_END
