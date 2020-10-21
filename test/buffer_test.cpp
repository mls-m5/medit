
#include "buffer.h"
#include "mls-unit-test/unittest.h"
#include <string_view>

TEST_SUIT_BEGIN

TEST_CASE("instantiate") {
    Buffer buffer;
}

TEST_CASE("fill with text") {
    const std::string_view testText = "apa bepa\n bearne";

    Buffer buffer{testText};

    auto returnText = buffer.text();

    ASSERT_EQ(returnText.size(), testText.size());
    ASSERT_EQ(returnText, testText);
}

TEST_CASE("remove character") {
    const std::string_view testText = "apa bepa\n bearne";
    const std::string_view resultText = "ap bepa\n bearne";

    Buffer buffer{testText};

    buffer.erase({.x = 2, .y = 0});

    ASSERT_EQ(buffer.text(), resultText);
}

TEST_SUIT_END
