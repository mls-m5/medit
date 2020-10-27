
#include "text/buffer.h"
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

    buffer.erase({.x = 3, .y = 0});

    ASSERT_EQ(buffer.text(), resultText);
}

TEST_CASE("remove character on second line") {
    const std::string_view testText = "apa bepa\n bearne";
    const std::string_view resultText = "apa bepa\n barne";

    Buffer buffer{testText};

    buffer.erase({.x = 3, .y = 1});

    ASSERT_EQ(buffer.text(), resultText);
}

TEST_CASE("remove at invalid cursor without crashing") {
    const std::string_view testText = "apa bepa\n bearne";

    auto buffer = Buffer{testText};
    buffer.erase({100, 0});
    buffer.erase({1, 100});
}

TEST_CASE("try to insert to empty buffer") {
    auto buffer = Buffer{};
    buffer.insert("a", {0, 0});
}

TEST_CASE("split newlines") {
    const auto testText = std::string_view{"apa bepa\n bearne"};
    auto buffer = Buffer{testText};

    ASSERT_EQ(buffer.lines().size(), 2);

    buffer.insert("\n", {10, 1});
    ASSERT_EQ(buffer.lines().size(), 3);
}

TEST_CASE("insert at position") {
    const auto testText = std::string_view{"apa bepa\n bearne"};
    const auto compText = std::string_view{"apa bepa\n boearne"};
    auto buffer = Buffer{testText};

    buffer.insert('o', {2, 1});
    ASSERT_EQ(buffer.text(), compText);
}

TEST_SUIT_END
