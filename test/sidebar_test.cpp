#include "files/file.h"
#include "mls-unit-test/unittest.h"
#include "mock/screen/mockscreen.h"
#include "text/buffer.h"
#include "views/bufferview.h"
#include "views/mainwindowlayout.h"
#include "views/sidebar.h"

TEST_SUIT_BEGIN(Sidebar)

TEST_CASE("sidebar reserves space without losing remainder columns") {
    auto layout = MainWindowLayout::calculate(100, 30, 2, true, false, 10);
    ASSERT_EQ(layout.sidebarWidth, 25u);
    ASSERT_EQ(layout.editorHeight, 29u);
    ASSERT_EQ(layout.editorX(0, 2), 25u);
    ASSERT_EQ(layout.editorColumns(0, 2), 38u);
    ASSERT_EQ(layout.editorX(1, 2), 63u);
    ASSERT_EQ(layout.editorColumns(1, 2), 37u);
    ASSERT_EQ(layout.consoleRows, 0u);
}

TEST_CASE("hidden sidebar gives all columns back to editors") {
    auto layout = MainWindowLayout::calculate(101, 30, 2, false, false, 10);
    ASSERT_EQ(layout.sidebarWidth, 0u);
    ASSERT_EQ(layout.editorX(0, 2), 0u);
    ASSERT_EQ(layout.editorColumns(0, 2), 51u);
    ASSERT_EQ(layout.editorX(1, 2), 51u);
    ASSERT_EQ(layout.editorColumns(1, 2), 50u);
}

TEST_CASE("console and status keep their full-width rows") {
    auto layout = MainWindowLayout::calculate(100, 30, 1, true, true, 10);
    ASSERT_EQ(layout.editorHeight, 20u);
    ASSERT_EQ(layout.consoleRows, 9u);
    ASSERT_EQ(layout.editorHeight + layout.consoleRows + 1, 30u);
    ASSERT_EQ(layout.sidebarWidth, 25u);
}

TEST_CASE("narrow windows temporarily hide the requested sidebar") {
    auto narrow = MainWindowLayout::calculate(40, 30, 2, true, false, 10);
    ASSERT_EQ(narrow.sidebarWidth, 0u);
    auto wide = MainWindowLayout::calculate(41, 30, 2, true, false, 10);
    ASSERT_EQ(wide.sidebarWidth, 25u);
    ASSERT_EQ(wide.editorColumns(0, 2), 8u);
    ASSERT_EQ(wide.editorColumns(1, 2), 8u);
}

TEST_CASE("small window dimensions stay bounded") {
    for (size_t width = 0; width < 50; ++width) {
        for (size_t height = 0; height < 15; ++height) {
            for (size_t count = 1; count < 5; ++count) {
                for (bool console : {false, true}) {
                    auto layout = MainWindowLayout::calculate(
                        width, height, count, true, console, 10);
                    EXPECT_TRUE(layout.sidebarWidth <= width);
                    const auto availableRows = height > 0 ? height - 1 : 0;
                    ASSERT_EQ(layout.editorHeight + layout.consoleRows,
                              availableRows);
                    size_t end = layout.sidebarWidth;
                    for (size_t index = 0; index < count; ++index) {
                        ASSERT_EQ(layout.editorX(index, count), end);
                        end += layout.editorColumns(index, count);
                    }
                    ASSERT_EQ(end, width);
                }
            }
        }
    }
}

TEST_CASE("wrapped text still makes progress in very narrow editor splits") {
    auto buffer = std::make_shared<Buffer>("abcdefgh");
    buffer->assignFile(std::make_unique<File>("notes.md"));
    BufferView view{nullptr, buffer};
    EXPECT_TRUE(view.shouldWrap());
    view.showLines(true);
    view.width(20);
    for (size_t width = 0; width <= 3; ++width) {
        view.width(width);
        // All eight characters occupy their own virtual line.
        ASSERT_EQ(view.cursorToLocal({7, 0}).y(), 7u);
    }
}

TEST_CASE("dummy title and separator stay inside the sidebar") {
    MockScreen screen;
    SideBar sidebar{nullptr};
    sidebar.x(2);
    sidebar.y(3);
    sidebar.width(25);
    sidebar.height(4);
    bool titleDrawn = false;
    size_t separators = 0;
    screen.mock_draw_3.expectNum(9);
    screen.mock_draw_3.expectArgs([&](auto x, auto y, FStringView text) {
        if (text == FString{" Project"}) {
            titleDrawn = x == 2 && y == 3;
        }
        if (text == FString{"|"} && x == 26) {
            ++separators;
        }
        return x >= 2 && x + text.size() <= 27 && y >= 3 && y < 7;
    });
    sidebar.draw(screen);
    ASSERT_EQ(titleDrawn, true);
    ASSERT_EQ(separators, 4u);
}

TEST_CASE("hidden and empty sidebars do not draw") {
    MockScreen screen;
    screen.mock_draw_3.expectNum(0);
    SideBar sidebar{nullptr};
    sidebar.width(25);
    sidebar.height(4);
    sidebar.visible(false);
    sidebar.draw(screen);
    sidebar.visible(true);
    sidebar.width(0);
    sidebar.draw(screen);
    sidebar.width(25);
    sidebar.height(0);
    sidebar.draw(screen);
}

TEST_CASE("dummy title is clipped in a small sidebar") {
    MockScreen screen;
    SideBar sidebar{nullptr};
    sidebar.width(4);
    sidebar.height(1);
    bool titleClipped = false;
    screen.mock_draw_3.expectNum(3);
    screen.mock_draw_3.expectArgs([&](auto x, auto y, FStringView text) {
        if (text == FString{" Pr"}) {
            titleClipped = true;
        }
        return x + text.size() <= 4 && y == 0;
    });
    sidebar.draw(screen);
    ASSERT_EQ(titleClipped, true);
}

TEST_SUIT_END
