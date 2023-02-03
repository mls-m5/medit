#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "text/history.h"
#include <string_view>

using namespace std::literals;

TEST_SUIT_BEGIN

TEST_CASE("simple undo") {
    auto testString1 = "hello there"s;
    auto testString2 = "wazzup?"s;

    auto buffer = Buffer{testString1};
    auto &history = buffer.history();

    //    history.commit();

    buffer.text(testString2);

    //    history.commit();

    history.undo();

    ASSERT_EQ(buffer.text(), testString1);

    history.redo();

    ASSERT_EQ(buffer.text(), testString2);
}

TEST_CASE("simple undo") {
    auto testString1 = "hello there"s;
    auto testString2 = "wazzup?"s;
    auto testString3 = "wazzupzz?"s;

    auto buffer = Buffer{testString1};
    auto &history = buffer.history();

    history.markMajor();

    buffer.text(testString2);

    history.markMajor();

    buffer.text(testString3);

    history.markMajor();

    history.undoMajor();

    ASSERT_EQ(buffer.text(), testString2);

    history.undoMajor();

    ASSERT_EQ(buffer.text(), testString1);

    history.redo();

    ASSERT_EQ(buffer.text(), testString2);

    history.undo();

    ASSERT_EQ(buffer.text(), testString1);
}

TEST_CASE("multi line undo") {
    auto testString1 = "apa\nbepa\ncepa\ndepa"s;

    auto buffer = Buffer{testString1};
    auto &history = buffer.history();

    history.markMajor();

    buffer.text("apa\nbe2pa\nc234epa\ndepa"s);

    history.markMajor();

    history.undoMajor();

    ASSERT_EQ(buffer.text(), testString1);
}

TEST_SUIT_END
