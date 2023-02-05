#include "mls-unit-test/unittest.h"
#include "mock/screen/mockscreen.h"
#include "screen/deserializescreen.h"
#include "screen/serializescreen.h"
#include "syntax/palette.h"
#include <future>

using namespace std::literals;

struct Fixture {
    std::shared_ptr<MockScreen> output = std::make_shared<MockScreen>();
    std::shared_ptr<DeserializeScreen> ds =
        std::make_shared<DeserializeScreen>(output);
    SerializeScreen ss{ds};
};

TEST_SUIT_BEGIN

TEST_CASE("draw(...)") {
    auto f = Fixture{};

    f.output->mock_draw_3.expectArgs(
        [](auto x, auto y, const FString &str) -> bool {
            return x == 10 && y == 20 && str == FString{"hello"};
        });
    f.output->mock_draw_3.expectNum(1);

    f.ss.draw(10, 20, "hello");
}

TEST_CASE("clear()") {
    auto fixture = Fixture{};

    fixture.output->mock_clear_0.expectNum(1);
    fixture.ss.clear();
}

TEST_CASE("refresh()") {
    auto fixture = Fixture{};

    fixture.output->mock_refresh_0.expectNum(1);
    fixture.ss.refresh();
}

TEST_CASE("cursor(...)") {
    auto f = Fixture{};

    f.output->mock_cursor_2.expectArgs([](auto x, auto y) -> bool {
        return x == 10 && y == 20; //
    });
    f.output->mock_cursor_2.expectNum(1);

    f.ss.cursor(10, 20);
}

TEST_CASE("x(), y()") {
    {
        auto f = Fixture{};

        auto future = std::async([&f] {
            f.output->mock_x_0.returnValue(13);
            EXPECT_EQ(f.ss.x(), 13);
        });
    }
    {
        auto f = Fixture{};

        auto future = std::async([&f] {
            f.output->mock_y_0.returnValue(14);
            EXPECT_EQ(f.ss.y(), 14);
        });
    }
}

TEST_CASE("width(), height()") {
    {
        auto f = Fixture{};

        auto future = std::async([&f] {
            f.output->mock_width_0.returnValue(13);
            EXPECT_EQ(f.ss.width(), 13);
        });
    }
    {
        auto f = Fixture{};

        auto future = std::async([&f] {
            f.output->mock_height_0.returnValue(14);
            EXPECT_EQ(f.ss.height(), 14);
        });
    }
}

TEST_CASE("title(...)") {
    auto f = Fixture{};

    f.output->mock_title_1.expectArgs([](auto x) -> bool {
        return x == "hello"; //
    });
    f.output->mock_title_1.expectNum(1);

    f.ss.title("hello");
}

TEST_CASE("cursorStyle(...)") {
    auto f = Fixture{};

    f.output->mock_cursorStyle_1.expectArgs([](auto x) -> bool {
        return x == CursorStyle::Beam; //
    });
    f.output->mock_cursorStyle_1.expectNum(1);

    f.ss.cursorStyle(CursorStyle::Beam);
}

TEST_CASE("palette(...)") {
    auto f = Fixture{};

    f.output->mock_palette_1.expectNum(1);

    auto palette = Palette{};
    f.ss.palette(palette);
}

TEST_SUIT_END
