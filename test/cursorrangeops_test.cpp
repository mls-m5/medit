
#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "text/cursorrangeops.h"

using namespace std::literals;

TEST_SUIT_BEGIN

TEST_CASE("single line content") {
    auto buffer = Buffer{"apa bepa\ncepa"sv};

    {
        auto text = content({buffer, {0, 0}, {2, 0}});

        ASSERT_EQ(text.size(), 1);
        ASSERT_EQ(std::string{text.front()}, "ap");
    }
    {
        auto text = content({buffer, {0, 0}, {8, 0}});

        ASSERT_EQ(text.size(), 1);
        ASSERT_EQ(std::string{text.front()}, "apa bepa");
    }
}

TEST_CASE("multi line content") {
    auto buffer = Buffer{"apa bepa\ncepa"sv};

    {
        auto text = content({buffer, {0, 0}, {8, 1}});

        ASSERT_EQ(text.size(), 2);
        ASSERT_EQ(std::string{text.front()}, "apa bepa");
        ASSERT_EQ(std::string{text.back()}, "cepa");
    }
    {
        auto text = content({buffer, {2, 0}, {8, 1}});

        ASSERT_EQ(text.size(), 2);
        ASSERT_EQ(std::string{text.front()}, "a bepa");
        ASSERT_EQ(std::string{text.back()}, "cepa");
    }
    {
        auto text = content({buffer, {0, 0}, {1, 1}});

        ASSERT_EQ(text.size(), 2);
        ASSERT_EQ(std::string{text.front()}, "apa bepa");
        ASSERT_EQ(std::string{text.back()}, "c");
    }
}

TEST_SUIT_END
