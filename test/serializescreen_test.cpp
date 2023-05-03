#include "mls-unit-test/unittest.h"
#include "mock/screen/mockscreen.h"
#include "screen/deserializescreen.h"
#include "screen/serializescreen.h"
#include "syntax/palette.h"
#include <future>

using namespace std::literals;

struct Fixture {
    int subscription() {
        output->mock_unsubscribe_0.expectNum(1);
        output->mock_subscribe_1.expectNum(1);
        output->mock_subscribe_1.onCall(
            [this](IScreen::CallbackT f) { _callback = f; });
        return 0;
    }

    void sendEvent(std::vector<Event> e) {
        _callback(std::move(e));
    }

    IScreen::CallbackT _callback;

    std::shared_ptr<MockScreen> output = std::make_shared<MockScreen>();
    int _sub = subscription(); // Just do it in right order
    std::shared_ptr<DeserializeScreen> ds =
        std::make_shared<DeserializeScreen>(output);
    SerializeScreen ss{ds};
};

TEST_SUIT_BEGIN

TEST_CASE("draw(...)") {
    auto f = Fixture{};

    f.output->mock_draw_3.expectArgs(
        [](auto x, auto y, FStringView str) -> bool {
            return x == 10 && y == 20 && str == FString{"hello"} &&
                   str.front().f == 12;
        });
    f.output->mock_draw_3.expectNum(1);

    f.ss.draw(10, 20, FString{"hello", 12});
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

TEST_CASE("clipboard(...)") {
    {
        auto f = Fixture{};

        f.output->mock_clipboardData_1.expectNum(1);
        f.output->mock_clipboardData_1.expectArgs([](auto x) -> bool {
            return x == "hello"; //
        });

        f.ss.clipboardData("hello");
    }
    {
        auto f = Fixture{};

        f.output->mock_clipboardData_0.expectNum(1);
        f.output->mock_clipboardData_0.onCall(
            []() -> std::string { return "hello2"; });

        EXPECT_EQ(f.ss.clipboardData(), "hello2");
    }
}

TEST_CASE("key callback") {
    {
        auto f = Fixture{};

        bool wasCalled = false;
        auto cb = [&wasCalled](auto &&) { wasCalled = true; };
        f.ss.subscribe(cb);

        f.sendEvent({KeyEvent{}});

        EXPECT(wasCalled);
    }
    {
        auto f = Fixture{};

        bool wasCalled = false;
        auto cb = [&wasCalled](auto &&list) {
            wasCalled = true;
            if (list.empty()) {
                return;
            }
            auto p = std::get_if<PasteEvent>(&list.front());
            EXPECT(p);
            EXPECT_EQ(p->text, "hello there");
        };
        f.ss.subscribe(cb);

        f.sendEvent({PasteEvent{"hello there"}});

        EXPECT(wasCalled);
    }

    {
        auto f = Fixture{};

        bool wasCalled = false;
        auto cb = [&wasCalled](auto &&list) {
            wasCalled = true;
            if (list.empty()) {
                return;
            }
            auto p = std::get_if<ResizeEvent>(&list.front());
            EXPECT(p);
            EXPECT_EQ(p->width, 100);
            EXPECT_EQ(p->height, 120);
        };
        f.ss.subscribe(cb);

        f.sendEvent({ResizeEvent{100, 120}});

        EXPECT(wasCalled);
    }
}
TEST_SUIT_END
