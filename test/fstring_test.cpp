
#define DO_NOT_CATCH_ERRORS

#include "mls-unit-test/unittest.h"
#include "text/fstring.h"

TEST_SUIT_BEGIN

TEST_CASE("create from std::string") {
    auto s = std::string{"hello där"};
    auto f = FString{s};

    ASSERT_EQ(f.size(), std::string("hello dar").size());
}

TEST_CASE("convert back to std::string") {
    auto s = std::string{"hello där"};
    auto f = FString{s};
    auto res = std::string{f};

    ASSERT_EQ(s, res);
}

TEST_CASE("split") {
    auto f = FString{"hello\nthere\n"};
    auto split = f.split('\n');

    ASSERT_EQ(split.size(), 3);

    ASSERT_EQ(split.at(0), FString{"hello"});
}

TEST_SUIT_END
