#include "keys/bufferkeymap.h"
#include "mls-unit-test/unittest.h"

//! To be able to print nullptr
std::ostream &operator<<(std::ostream &s, std::nullptr_t) {
    return s << static_cast<void *>(nullptr);
}

TEST_SUIT_BEGIN

TEST_CASE("create") {
    auto map = BufferKeyMap{BufferKeyMap::MapType{
        {
            {"hello"},
            {""},
        },
    }};
}

TEST_CASE("no match") {
    auto map = BufferKeyMap{BufferKeyMap::MapType{
        {
            {"hello"},
            {""},
        },
    }};

    auto m = map.match("apa");

    ASSERT_EQ(m.first, map.NoMatch);
    ASSERT_EQ(m.second, nullptr);
}

TEST_CASE("partial match") {
    auto map = BufferKeyMap{BufferKeyMap::MapType{
        {
            {"hello"},
            {""},
        },
    }};

    auto m = map.match("hel");

    ASSERT_EQ(m.first, map.PartialMatch);
    ASSERT_EQ(m.second, nullptr);
}

TEST_CASE("match") {
    auto map = BufferKeyMap{BufferKeyMap::MapType{
        {
            {"hello"},
            {""},
        },
    }};

    auto m = map.match("hello");

    ASSERT_EQ(m.first, map.Match);
    ASSERT_NE(m.second, nullptr);
}

TEST_SUIT_END
