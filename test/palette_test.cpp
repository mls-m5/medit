
#include "mls-unit-test/unittest.h"
#include "syntax/color.h"
#include "syntax/palette.h"
#include <sstream>

using namespace std::literals;

TEST_SUIT_BEGIN

TEST_CASE("color") {
    auto color = Color{"#abcdef"};
    

    ASSERT_EQ(color.r(), 0xab);
    ASSERT_EQ(color.g(), 0xcd);
    ASSERT_EQ(color.b(), 0xef);

    // Just do not crash
    Color{"öab"};
}

TEST_CASE("create") {
    auto testJson = R"_(
            {
               "palette": {
                  "hello": "#abcdef"
               },
               "style": {
                  "standard": {"foreground": "hello"}
               }
            }
     )_"s;

    auto palette = Palette{};

    std::stringstream(testJson) >> palette;

    {
        auto color = palette.getColor("hello");
        ASSERT_EQ(color.r(), 0xab);
        ASSERT_EQ(color.g(), 0xcd);
        ASSERT_EQ(color.b(), 0xef);
    }

    {
        auto color = palette.getStyleColor("standard");
        ASSERT_EQ(color.r(), 0xab);
        ASSERT_EQ(color.g(), 0xcd);
        ASSERT_EQ(color.b(), 0xef);
    }
}

TEST_SUIT_END