
#define DO_NOT_CATCH_ERRORS

#include "mls-unit-test/unittest.h"
#include "text/fstring.h"
#include "text/fstringview.h"

TEST_SUIT_BEGIN(FString)

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

TEST_CASE("fstring view") {
    auto f = FString{"hello\nthere\n"};
    auto f2 = FString{"hello\nthere\nyou"};

    auto fsv = FStringView{f};
    auto fsv2 = FStringView{f2};

    ASSERT_EQ(f, fsv);
    ASSERT_EQ(fsv, f);
    ASSERT_EQ(fsv, fsv);
    ASSERT_NE(fsv, fsv2);
    ASSERT_NE(fsv, f2);
    ASSERT_NE(f, fsv2);
}

TEST_SUIT_END
