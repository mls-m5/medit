
#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "text/cursorrangeops.h"

using namespace std::literals;

TEST_SUIT_BEGIN

TEST_CASE("single line content") {
    auto buffer = Buffer{"apa bepa\ncepa"sv};

    {
        auto text = contentLines({buffer, {0, 0}, {2, 0}});

        ASSERT_EQ(text.size(), 1);
        ASSERT_EQ(std::string{text.front()}, "ap");
    }
    {
        auto text = contentLines({buffer, {0, 0}, {8, 0}});

        ASSERT_EQ(text.size(), 1);
        ASSERT_EQ(std::string{text.front()}, "apa bepa");
    }
}

TEST_CASE("multi line content") {
    auto buffer = Buffer{"apa bepa\ncepa"sv};

    {
        auto text = contentLines({buffer, {0, 0}, {8, 1}});

        ASSERT_EQ(text.size(), 2);
        ASSERT_EQ(std::string{text.front()}, "apa bepa");
        ASSERT_EQ(std::string{text.back()}, "cepa");
    }
    {
        auto text = contentLines({buffer, {2, 0}, {8, 1}});

        ASSERT_EQ(text.size(), 2);
        ASSERT_EQ(std::string{text.front()}, "a bepa");
        ASSERT_EQ(std::string{text.back()}, "cepa");
    }
    {
        auto text = contentLines({buffer, {0, 0}, {1, 1}});

        ASSERT_EQ(text.size(), 2);
        ASSERT_EQ(std::string{text.front()}, "apa bepa");
        ASSERT_EQ(std::string{text.back()}, "c");
    }
}

TEST_CASE("erase empty range") {
    const auto testText = "apa bepa\ncepa"sv;

    // Do nothing  with empty range
    auto buffer = Buffer{testText};
    buffer.isChanged(false);

    auto cursor = erase({buffer, {1, 0}, {1, 0}});

    ASSERT_EQ(buffer.text(), testText);
    ASSERT_EQ(cursor.x(), 1);
    ASSERT_EQ(cursor.y(), 0);
    ASSERT_EQ(buffer.isChanged(), false);
}

TEST_CASE("erase range - single line") {
    const auto testText = "ape beba\ncepa\ndepo"sv;
    const auto resText = "aeba\ncepa\ndepo"sv;
    auto buffer = Buffer{testText};

    auto cursor = erase({buffer, {1, 0}, {5, 0}});

    ASSERT_EQ(buffer.text(), resText);
    ASSERT_EQ(cursor.x(), 1);
    ASSERT_EQ(cursor.y(), 0);
}

TEST_CASE("erase range - multiline") {
    const auto testText = "apa bepa\ncepa\ndepo"sv;
    const auto resText = "apo"sv;
    auto buffer = Buffer{testText};

    auto cursor = erase({buffer, {1, 0}, {2, 2}});

    ASSERT_EQ(buffer.text(), resText);
    ASSERT_EQ(buffer.lines().size(), 1);
    ASSERT_EQ(cursor.x(), 1);
    ASSERT_EQ(cursor.y(), 0);
}

TEST_CASE("format range") {
    const auto testText = "apa bepa\ncepa\ndepo"sv;

    auto buffer = Buffer{testText};

    auto range = CursorRange{buffer, {1, 0}, {2, 1}};

    format(range, 4);

    for (auto c : range) {
        if (c) {
            ASSERT_EQ(c->f, 4);
        }
    }

    ASSERT_EQ(buffer.begin()->f, 1);
}

TEST_CASE("operator ==") {
    auto buffer = Buffer{"hello"sv};
    auto range = CursorRange{buffer.begin(), buffer.end()};

    auto res = range == "hello";
    auto res2 = range == "there";
    auto res3 = range == "hellooo";
    auto res4 = range == "hell";

    ASSERT_EQ(res, true);
    ASSERT_EQ(res2, false);
    ASSERT_EQ(res3, false);
    ASSERT_EQ(res4, false);
}

TEST_CASE("toString()") {
    auto testString = "hello\there"sv;
    auto buffer = Buffer{testString};
    auto range = CursorRange{buffer};
    ASSERT_EQ(testString, toString(range));
}

TEST_CASE("for each") {
    auto testString = "int main() {\n\n}"sv;
    auto buffer = Buffer{testString};

    auto range = CursorRange(buffer);

    size_t count = 0;

    for (auto c : range) {
        (void)c;
        ++count;
    }

    ASSERT_EQ(count, testString.size());
}

TEST_CASE("for each - fixed range") {
    auto testString = "int main() {\n\n}"sv;
    auto buffer = Buffer{testString};

    auto range = CursorRange(buffer);
    auto newEnd = range.end().y(100);
    range = CursorRange{range.begin(), newEnd};
    size_t count = 0;

    range = fix(range);

    for (auto c : range) {
        (void)c;
        ++count;
    }

    ASSERT_EQ(count, testString.size());
}

TEST_CASE("word(...)") {
    const auto testText = "hello there you"sv;
    const auto resText1 = "hello"sv;
    const auto resText2 = "there"sv;
    const auto resText3 = "you"sv;

    auto buffer = Buffer{testText};

    {
        auto cursor = Cursor{buffer, 1, 0};
        auto result = word(cursor);
        ASSERT_EQ(result, resText1);
    }
    {
        auto cursor = Cursor{buffer, 8, 0};
        auto result = word(cursor);
        ASSERT_EQ(result, resText2);
    }
    {
        auto cursor = Cursor{buffer, 14, 0};
        auto result = word(cursor);
        ASSERT_EQ(result, resText3);
    }
}

TEST_CASE("line(...)") {
    const auto testText = "so thats when i said\n"
                          "hello there you\n"
                          "wazzup\n"sv;

    auto buffer = Buffer{testText};

    {
        const auto resText = "so thats when i said"sv;
        auto cursor = Cursor{buffer, 3, 0};
        auto result = line(cursor);
        ASSERT_EQ(result, resText);
    }
    {
        const auto resText = "hello there you"sv;
        auto cursor = Cursor{buffer, 8, 1};
        auto result = line(cursor);
        ASSERT_EQ(result, resText);
    }
    {
        const auto resText = "wazzup"sv;
        auto cursor = Cursor{buffer, 0, 2};
        auto result = line(cursor);
        ASSERT_EQ(result, resText);
    }
}

TEST_CASE("inner(..., '(', ')')") {
    const auto testText = "int apa(float bepa);"sv;
    const auto resText1 = "float bepa"sv;

    auto buffer = Buffer{testText};

    {
        auto cursor = Cursor{buffer, 8, 0};
        auto result = inner(cursor, '(', ')');
        ASSERT_EQ(result, resText1);
    }
    {
        auto cursor = Cursor{buffer, 1, 0};
        auto result = inner(cursor, '(', ')');
        EXPECT_TRUE(result.empty());
    }
}

TEST_CASE("inner(..., '\"', '\"')") {
    const auto testText = "auto c = \"hello there\";"sv;
    const auto resText1 = "hello there"sv;

    auto buffer = Buffer{testText};

    {
        auto cursor = Cursor{buffer, 12, 0};
        auto result = inner(cursor, '"', '"');
        ASSERT_EQ(result, resText1);
    }
    {
        auto cursor = Cursor{buffer, 1, 0};
        auto result = inner(cursor, '"', '"');
        EXPECT_TRUE(result.empty());
    }
}

TEST_SUIT_END
