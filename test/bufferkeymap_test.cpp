#include "keys/bufferkeymap.h"
#include "mls-unit-test/unittest.h"

TEST_SUIT_BEGIN

TEST_CASE("create") {
    auto map = BufferKeyMap{BufferKeyMap::MapType{
        {
            {"hello"},
            {""},
        },
    }};
}

TEST_CASE("match whole") {
    auto map = BufferKeyMap{BufferKeyMap::MapType{
        {
            {"hello"},
            {""},
        },
    }};

    auto m = map.match("hello");

    ASSERT_EQ(m, map.Match);
}

TEST_CASE("partial match") {
    auto map = BufferKeyMap{BufferKeyMap::MapType{
        {
            {"hello"},
            {""},
        },
    }};

    auto m = map.match("hel");

    ASSERT_EQ(m, map.PartialMatch);
}

TEST_SUIT_END
