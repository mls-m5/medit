#include "files/file.h"
#include "mls-unit-test/expect.h"
#include "mls-unit-test/unittest.h"
#include "script/jumplist.h"
#include "text/cursor.h"
#include "text/cursorrangeops.h"
#include "text/position.h"
#include "views/editor.h"
#include <memory>
#include <string_view>

namespace jumplist {

struct Fixture {
    std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>();
    std::shared_ptr<Editor> editor = std::make_shared<Editor>(nullptr, buffer);
    JumpList jumpList;

    Fixture(std::string_view text) {
        auto file = std::make_shared<File>("/tmp/test.txt");
        editor->file();
        replace(editor->cursor(), text);
    }

    void update() {
        jumpList.updatePosition(editor);
    }

    void goBack() {
        auto back = jumpList.back();
        if (back) {
            auto pos = back->pos;
            auto editor = back->editor.lock();
            if (editor) {
                editor->cursor(pos);
            }
        }
    }

    void goForward() {
        auto forward = jumpList.forward();
        if (forward) {
            auto pos = forward->pos;
            auto editor = forward->editor.lock();
            if (editor) {
                editor->cursor(pos);
            }
        }
    }

    Cursor cursor() {
        return editor->cursor();
    }

    void moveTo(Position pos) {
        editor->cursor(pos);
        update();
    }
};

} // namespace jumplist

TEST_SUIT_BEGIN(JumpList)

using namespace jumplist;

TEST_CASE("basic") {
    auto text = R"_(
hello
there
you
howdy
dody
)_";

    auto fixture = Fixture{text};

    fixture.moveTo({0, 1});
    fixture.moveTo({0, 2});
    fixture.moveTo({0, 4});

    EXPECT_EQ(fixture.cursor(), Position(0, 4));

    fixture.goBack();
    EXPECT_EQ(fixture.cursor(), Position(0, 2));

    // Cannot go before first position
    fixture.goBack();
    EXPECT_EQ(fixture.cursor(), Position(0, 2));

    // Move the current position
    fixture.moveTo({0, 3});
    EXPECT_EQ(fixture.cursor(), Position(0, 3));

    fixture.goForward();
    EXPECT_EQ(fixture.cursor(), Position(0, 4));

    // Cannot go past last position
    fixture.goForward();
    EXPECT_EQ(fixture.cursor(), Position(0, 4));
}

TEST_SUIT_END
