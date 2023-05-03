#include "keys/bufferkeymap.h"
#include "mls-unit-test/unittest.h"

TEST_SUIT_BEGIN

TEST_CASE("create") {
    auto map = BufferKeyMap{BufferKeyMap::MapType{
        {
            {"hello"},
            {},
        },
    }};
}

TEST_CASE("no match") {
    auto map = BufferKeyMap{BufferKeyMap::MapType{
        {
            {"hello"},
            {},
        },
    }};

    auto m = map.match(FString{"apa"});

    ASSERT_EQ(m.first, map.NoMatch);
    EXPECT(!m.second);
}

TEST_CASE("partial match") {
    auto map = BufferKeyMap{BufferKeyMap::MapType{
        {
            {"hello"},
            {},
        },
    }};

    auto m = map.match(FString{"hel"});

    ASSERT_EQ(m.first, map.PartialMatch);
    EXPECT(!m.second);
}

TEST_CASE("match") {
    auto map = BufferKeyMap{BufferKeyMap::MapType{
        {
            {"hello"},
            {},
        },
    }};

    auto m = map.match(FString{"hello"});

    ASSERT_EQ(m.first, map.Match);
    EXPECT(m.second);
}

TEST_SUIT_END
