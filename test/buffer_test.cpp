#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include <string_view>

TEST_SUIT_BEGIN

TEST_CASE("instantiate") {
    Buffer buffer;
}

TEST_CASE("fill with text") {
    const std::string_view testText = "apa bepa\n bearne";

    Buffer buffer{testText};

    auto returnText = buffer.text();

    ASSERT_EQ(returnText, testText);
    ASSERT_EQ(returnText.size(), testText.size());
}

TEST_CASE("insert string single line") {
    const std::string_view testText = "apa bepa";
    {
        Buffer buffer{"apa"};
        buffer.singleLine(true);

        buffer.insert(0, testText);

        ASSERT_EQ(buffer.text(), "apa bepaapa");
    }
    {
        Buffer buffer{"apa"};
        buffer.singleLine(true);

        buffer.insert(1, testText);

        ASSERT_EQ(buffer.text(), "apaapa bepa");
    }
}

TEST_SUIT_END
