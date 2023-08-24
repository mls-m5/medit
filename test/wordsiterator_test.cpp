#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "text/cursorrangeops.h"
#include "text/words.h"

using namespace std::literals;

TEST_SUIT_BEGIN(WordsIterator)

TEST_CASE("count words") {
    auto buffer = Buffer{"hello there you"sv};

    size_t count = 0;
    for (auto word : Words(buffer)) {
        (void)word;
        ++count;
    }

    ASSERT_EQ(count, 3);
}

TEST_CASE("content check") {
    auto buffer = Buffer{"hello there you"sv};

    auto range = Words{buffer};

    auto it = range.begin();

    ASSERT_EQ(*it, "hello"sv);
    ASSERT_EQ(*(++it), "there"sv);
    ASSERT_EQ(*(++it), "you"sv);
}

TEST_CASE("space before") {
    auto buffer = Buffer{"  hello there you"sv};

    auto range = Words{buffer};

    auto it = range.begin();

    ASSERT_EQ(*it, "hello"sv);
}

TEST_CASE("space after single word") {
    auto buffer = Buffer{"hello   "sv};

    auto words = Words(buffer);
    auto it = words.begin();
    ++it;

    EXPECT(!(it != words.end()));
}

TEST_CASE("space after") {
    auto buffer = Buffer{"hello there you   "sv};

    size_t count = 0;
    for (auto word : Words(buffer)) {
        (void)word;
        ++count;
    }

    ASSERT_EQ(count, 3);
}

TEST_SUIT_END
