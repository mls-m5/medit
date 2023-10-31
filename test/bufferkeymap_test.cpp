#include "keys/bufferkeymap.h"
#include "mls-unit-test/expect.h"
#include "mls-unit-test/unittest.h"
#include "text/fstringview.h"

TEST_SUIT_BEGIN(BufferKeyMap)

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
            {"hello", [](auto) {}},
            {},
        },
    }};

    auto m = map.match(FString{"hello"});

    ASSERT_EQ(m.first, map.Match);
    EXPECT(m.second);
}

TEST_CASE("custom function") {
    auto map = BufferKeyMap{BufferKeyMap::MapType{}};

    {
        auto m = map.match(FString{"hello"});
        ASSERT_EQ(m.first, map.NoMatch);
        EXPECT_FALSE(m.second);
    }

    map.customMatchFunction([](FStringView str) -> BufferKeyMap::ReturnT {
        return {BufferKeyMap::Match, [](auto) {}};
    });

    {
        auto m = map.match(FString{"hello"});
        ASSERT_EQ(m.first, map.Match);
        EXPECT_TRUE(m.second);
    }
}

TEST_SUIT_END
