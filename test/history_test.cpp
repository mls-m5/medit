#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "text/history.h"
#include <string_view>

TEST_SUIT_BEGIN

TEST_CASE("instantiate") {
    auto history = History{};
}

TEST_CASE("simple undo") {
    auto history = History{};

    auto testString1 = "hello there";
    auto testString2 = "wazzup?";

    auto buffer = Buffer{testString1};

    history.commit(buffer);

    buffer = testString2;

    history.commit(buffer);

    history.undo(buffer);

    ASSERT_EQ(buffer.text(), testString1);

    history.redo(buffer);

    ASSERT_EQ(buffer.text(), testString2);
}

TEST_CASE("multi line undo") {
    auto history = History{};

    auto testString1 = "apa\nbepa\ncepa\ndepa";

    auto buffer = Buffer{testString1};

    history.commit(buffer);

    buffer = "apa\nbe2pa\nc234epa\ndepa";

    history.commit(buffer);

    history.undo(buffer);

    ASSERT_EQ(buffer.text(), testString1);
}

TEST_SUIT_END
