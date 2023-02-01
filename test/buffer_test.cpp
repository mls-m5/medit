#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include <string_view>

using namespace std::literals;

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

TEST_CASE("apply text change") {
    {
        auto testText = "hello there"sv;
        auto resultText = "wazzup?"sv;

        Buffer buffer{testText};

        buffer.apply(BufferEdit{
            "hello there",
            "wazzup?",
            buffer.cursor({0, 0}),
        });

        auto returnText = buffer.text();

        ASSERT_EQ(returnText, resultText);
        ASSERT_EQ(returnText.size(), resultText.size());
    }

    {
        const std::string_view testText = "apa bepa\n bearne";
        const std::string_view resultText = "bepa bepa\n bearne";

        Buffer buffer{testText};

        buffer.apply(BufferEdit{
            "apa",
            "bepa",
            buffer.cursor({0, 0}),
        });

        auto returnText = buffer.text();

        ASSERT_EQ(returnText, resultText);
        ASSERT_EQ(returnText.size(), resultText.size());
    }
}

TEST_CASE("apply text change - multiline") {

    {
        const std::string_view testText = "apa bepa\n bearne";
        const std::string_view resultText = "ax bearne";

        Buffer buffer{testText};

        buffer.apply(BufferEdit{
            "pa bepa\n",
            "x",
            buffer.cursor({1, 0}),
        });

        auto returnText = buffer.text();

        ASSERT_EQ(returnText, resultText);
        ASSERT_EQ(returnText.size(), resultText.size());
    }

    {
        const std::string_view testText = "apa bepa\n bearne";
        const std::string_view resultText = "x bearne";

        Buffer buffer{testText};

        buffer.apply(BufferEdit{
            "apa bepa\n",
            "x",
            buffer.cursor({0, 0}),
        });

        auto returnText = buffer.text();

        ASSERT_EQ(returnText, resultText);
        ASSERT_EQ(returnText.size(), resultText.size());
    }
}

TEST_SUIT_END
