
#include "mls-unit-test/unittest.h"
#include "text/utf8char.h"

TEST_SUIT_BEGIN

TEST_CASE("single character char") {
    {
        constexpr auto c = Utf8Char{"a", 1};

        static_assert(c.size() == 1);
    }

    {
        constexpr auto c = Utf8Char{"a"};

        static_assert(c.size() == 1);
    }
}

TEST_CASE("double character char") {
    // example characters
    // https://design215.com/toolbox/ascii-utf8.php
    {
        constexpr auto c = Utf8Char{"å", 2};

        static_assert(c.size() == 2);
    }

    {
        constexpr auto c = Utf8Char{"å"};

        static_assert(c.size() == 2);
    }
}

TEST_CASE("double character char") {
    {
        constexpr auto c = Utf8Char{"இ", 3};

        static_assert(c.size() == 3);
    }

    {
        constexpr auto c = Utf8Char{"இ"};

        static_assert(c.size() == 3);
    }
}

TEST_CASE("double character char") {
    {
        constexpr auto c = Utf8Char{"🃘", 4};

        static_assert(c.size() == 4);
    }

    {
        constexpr auto c8 = Utf8Char{"🃘"};

        static_assert(c8.size() == 4);
    }
}

TEST_SUIT_END
