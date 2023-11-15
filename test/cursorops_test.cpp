
#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "text/cursorops.h"

using namespace std::literals;

TEST_SUIT_BEGIN(CursorOps)

TEST_CASE("remove character") {
    const std::string_view testText = "apa bepa\n bearne";
    const std::string_view resultText = "ap bepa\n bearne";

    Buffer buffer{testText};

    erase({buffer, 3, 0});

    ASSERT_EQ(buffer.text(), resultText);
}

TEST_CASE("insert") {
    const std::string_view testText = "apa";
    const std::string_view resultText = "apna";

    Buffer buffer{testText};

    auto cur = insert('n', {buffer, 2, 0});

    ASSERT_EQ(buffer.text(), resultText);
    ASSERT_EQ(cur.x(), 3);
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
        deleteLine({buffer, 1, 1});
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

TEST_CASE("insert string") {
    const auto testText = std::string_view{"apa bepa"};
    const auto resText1 = std::string_view{"apa-cepa bepa"};

    {
        auto buffer = Buffer{testText};
        auto cursor = insert({buffer, 3, 0}, "-cepa");
        ASSERT_EQ(buffer.text(), resText1);
        ASSERT_EQ(cursor.x(), 8);
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

TEST_CASE("goto beginning of _next_ word") {
    const auto testText = "Apa bepa\ncepa"sv;

    auto buffer = Buffer{testText};
    {
        auto c = nextWord({buffer, 0, 0});

        ASSERT_EQ(c.x(), 4);
        ASSERT_EQ(c.y(), 0);
    }
    {
        auto c = nextWord({buffer, 2, 0});

        ASSERT_EQ(c.x(), 4);
        ASSERT_EQ(c.y(), 0);
    }
    {
        auto c = nextWord({buffer, 3, 0});

        ASSERT_EQ(c.x(), 4);
        ASSERT_EQ(c.y(), 0);
    }
    {
        auto c = nextWord({buffer, 5, 0});

        ASSERT_EQ(c.x(), 0);
        ASSERT_EQ(c.y(), 1);
    }
    {
        // On the newline character
        auto c = nextWord({buffer, 8, 0});

        ASSERT_EQ(c.x(), 0);
        ASSERT_EQ(c.y(), 1);
    }
    {
        // I guess this differs from vim, but in practice it should be the same
        auto c = nextWord({buffer, 2, 1});

        ASSERT_EQ(c.x(), 4); // The character passed the last char in the buffer
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

TEST_CASE("for each") {
    auto buffer = Buffer{"int main() {\n\n}"};

    for (auto c : buffer) {
        (void)c;
    }
}

TEST_CASE("find()") {
    auto buffer = Buffer{"apa bepa cepa depa"};

    {
        auto f = find({buffer, 0, 0}, "d");
        EXPECT_TRUE(f);
        ASSERT_EQ(f->x(), 14);
        ASSERT_EQ(f->y(), 0);
    }

    {
        // If not found return same as before
        auto f = find({buffer, 14, 0}, "b");
        EXPECT_FALSE(f);
    }
}

TEST_CASE("find() - multiline") {
    auto buffer = Buffer{"apa bepa\ncepa depa"};

    {
        auto f = find({buffer, 0, 0}, "d", true);
        EXPECT_TRUE(f);
        ASSERT_EQ(f->x(), 5);
        ASSERT_EQ(f->y(), 1);
    }

    {
        // If not found return same as before
        auto f = find({buffer, 8, 0}, "b", true);
        EXPECT_FALSE(f);
    }
}

TEST_CASE("findMatching() ") {
    auto buffer = Buffer{"apa( bepa\ncepa )depa"};

    {
        auto cursor = findMatching({buffer, 3, 0});
        ASSERT_EQ(cursor.x(), 5);
        ASSERT_EQ(cursor.y(), 1);
    }
    {
        auto cursor = findMatching({buffer, 5, 1});
        ASSERT_EQ(cursor.x(), 3);
        ASSERT_EQ(cursor.y(), 0);
    }
}

TEST_CASE("rfind()") {
    auto buffer = Buffer{"apa bepa cepa depa"};

    {
        auto f = rfind({buffer, 14, 0}, "b");
        EXPECT_TRUE(f);
        ASSERT_EQ(f->x(), 4);
        ASSERT_EQ(f->y(), 0);
    }

    {
        // If not found return same as before
        auto f = rfind({buffer, 5, 0}, "d");
        EXPECT_FALSE(f);
    }
}

TEST_CASE("rfind() - multiline") {
    auto buffer = Buffer{"apa bepa\ncepa depa"};

    {
        auto f = rfind({buffer, 2, 1}, "b", true);
        EXPECT_TRUE(f);
        ASSERT_EQ(f->x(), 4);
        ASSERT_EQ(f->y(), 0);
    }

    {
        auto f = rfind({buffer, 5, 0}, "d", true);
        EXPECT_FALSE(f);
    }
}

TEST_SUIT_END
