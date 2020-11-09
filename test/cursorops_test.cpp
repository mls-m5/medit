
#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "text/cursorops.h"

using namespace std::literals;

TEST_SUIT_BEGIN

TEST_CASE("remove character") {
    const std::string_view testText = "apa bepa\n bearne";
    const std::string_view resultText = "ap bepa\n bearne";

    Buffer buffer{testText};

    erase({buffer, 3, 0});

    ASSERT_EQ(buffer.text(), resultText);
}

TEST_CASE("remove character on second line") {
    const std::string_view testText = "apa bepa\n bearne";
    const std::string_view resultText = "apa bepa\n barne";

    Buffer buffer{testText};

    erase({buffer, 3, 1});

    ASSERT_EQ(buffer.text(), resultText);
}

TEST_CASE("remove to join lines") {

    const std::string_view testText = "apa bepa\n bearne";
    const std::string_view resultText = "apa bepa bearne";

    Buffer buffer{testText};

    erase({buffer, 0, 1});

    ASSERT_EQ(buffer.text(), resultText);
}

TEST_CASE("remove at invalid cursor without crashing") {
    const std::string_view testText = "apa bepa\n bearne";

    auto buffer = Buffer{testText};
    erase({buffer, 100, 0});
    erase({buffer, 1, 100});
}

TEST_CASE("try to insert to empty buffer") {
    auto buffer = Buffer{};
    insert("a", {buffer, 0, 0});
}

TEST_CASE("split newlines") {
    const auto testText = std::string_view{"apa bepa\n bearne"};
    auto buffer = Buffer{testText};

    ASSERT_EQ(buffer.lines().size(), 2);

    insert("\n", {buffer, 10, 1});
    ASSERT_EQ(buffer.lines().size(), 3);
}

TEST_CASE("insert at position") {
    const auto testText = std::string_view{"apa bepa\n bearne"};
    const auto compText = std::string_view{"apa bepa\n boearne"};
    auto buffer = Buffer{testText};

    insert('o', {buffer, 2, 1});
    ASSERT_EQ(buffer.text(), compText);
}

TEST_CASE("split line with insert newline") {
    const auto testText = "apabepa"sv;
    const auto resText = "apa\nbepa"sv;
    auto buffer = Buffer{testText};

    insert('\n', {buffer, 3, 0});
    ASSERT_EQ(buffer.text(), resText);
}

TEST_CASE("do not remove anything when on first character") {
    const auto testText = std::string_view{"apa bepa\n bearne"};
    auto buffer = Buffer{testText};
    erase({buffer, 0, 0});
    ASSERT_EQ(buffer.text(), testText);
}

TEST_CASE("delete line") {
    const auto testText = std::string_view{"apa bepa\n bearne\ncepa"};
    const auto resText1 = std::string_view{"apa bepa\ncepa"};
    const auto resText2 = std::string_view{"apa bepa\n bearne"};
    {
        auto buffer = Buffer{testText};
        deleteLine({buffer, 0, 1});
        ASSERT_EQ(buffer.text(), resText1);
    }
    {
        auto buffer = Buffer{testText};
        deleteLine({buffer, 0, 3});
        ASSERT_EQ(buffer.text(), resText2);
    }
}

TEST_CASE("join lines") {
    const auto testText = std::string_view{"apa bepa\n bearne\ncepa"};
    const auto resText1 = std::string_view{"apa bepa\n bearnecepa"};

    {
        auto buffer = Buffer{testText};
        auto cursor = join({buffer, 0, 1});
        ASSERT_EQ(buffer.text(), resText1);
        ASSERT_EQ(cursor.x(), 7);
    }
    {
        auto buffer = Buffer{testText};
        join({buffer, 0, 4});
        ASSERT_EQ(buffer.text(), testText);
    }
}

TEST_CASE("split line") {
    const auto testText = std::string_view{"apa bepa\n bearne\ncepa"};
    const auto resText1 = std::string_view{"apa\n bepa\n bearne\ncepa"};
    const auto resText2 = std::string_view{"apa bepa\n\n bearne\ncepa"};
    const auto resText3 = std::string_view{"apa bepa\n bearne\ncepa\n"};
    {
        auto buffer = Buffer{testText};
        split({buffer, 3, 0});
        ASSERT_EQ(buffer.text(), resText1);
    }
    {
        auto buffer = Buffer{testText};
        split({buffer, 8, 0});
        ASSERT_EQ(buffer.text(), resText2);
    }
    {
        auto buffer = Buffer{testText};
        split({buffer, 8, 2});
        ASSERT_EQ(buffer.text(), resText3);
    }
}

TEST_CASE("split empty buffer without crashing") {
    auto buffer = Buffer{};
    split({buffer, 10, 10});
}

TEST_CASE("copy indentation") {
    const auto testText = "   \na"sv;
    const auto resText = "   \n   a"sv;
    auto buffer = Buffer{testText};

    auto cursor = copyIndentation({buffer, 0, 1});

    ASSERT_EQ(buffer.text(), resText);
    ASSERT_EQ(cursor.x(), 3);
}

TEST_CASE("get current character") {
    const auto testText = "Apa bepa\ncepa"sv;

    auto buffer = Buffer{testText};

    {
        auto c = content({buffer, 0, 0});
        ASSERT_EQ(c, 'A');
    }
    {
        auto c = content({buffer, 8, 0});
        ASSERT_EQ(c, '\n');
    }
    {
        auto c = content({buffer, 100, 100});
        ASSERT_EQ(c, '\n');
    }
}

TEST_CASE("goto beginning of word") {
    const auto testText = "Apa bepa\ncepa"sv;

    auto buffer = Buffer{testText};
    {
        auto c = wordBegin({buffer, 0, 0});

        ASSERT_EQ(c.x(), 0);
        ASSERT_EQ(c.y(), 0);
    }
    {
        auto c = wordBegin({buffer, 2, 0});

        ASSERT_EQ(c.x(), 0);
        ASSERT_EQ(c.y(), 0);
    }
    {
        auto c = wordBegin({buffer, 3, 0});

        ASSERT_EQ(c.x(), 0);
        ASSERT_EQ(c.y(), 0);
    }
    {
        auto c = wordBegin({buffer, 5, 0});

        ASSERT_EQ(c.x(), 4);
        ASSERT_EQ(c.y(), 0);
    }
    {
        // On the newline character
        auto c = wordBegin({buffer, 8, 0});

        ASSERT_EQ(c.x(), 4);
        ASSERT_EQ(c.y(), 0);
    }
    {
        auto c = wordBegin({buffer, 2, 1});

        ASSERT_EQ(c.x(), 0);
        ASSERT_EQ(c.y(), 1);
    }
}

TEST_CASE("goto end of word") {
    const auto testText = "Apa bepa\ncepa"sv;

    auto buffer = Buffer{testText};
    {
        auto c = wordEnd({buffer, 0, 0});

        ASSERT_EQ(c.x(), 2);
        ASSERT_EQ(c.y(), 0);
    }
    {
        auto c = wordEnd({buffer, 2, 0});

        ASSERT_EQ(c.x(), 2);
        ASSERT_EQ(c.y(), 0);
    }
    {
        auto c = wordEnd({buffer, 3, 0});

        ASSERT_EQ(c.x(), 7);
        ASSERT_EQ(c.y(), 0);
    }
    {
        auto c = wordEnd({buffer, 5, 0});

        ASSERT_EQ(c.x(), 7);
        ASSERT_EQ(c.y(), 0);
    }
    {
        // On the newline character
        auto c = wordEnd({buffer, 8, 0});

        ASSERT_EQ(c.x(), 3);
        ASSERT_EQ(c.y(), 1);
    }
    {
        auto c = wordEnd({buffer, 2, 1});

        ASSERT_EQ(c.x(), 3);
        ASSERT_EQ(c.y(), 1);
    }
}
TEST_SUIT_END
