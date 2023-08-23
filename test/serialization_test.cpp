#include "mls-unit-test/unittest.h"
// #include "text/fchar_serialization.h"
// #include "text/fstring_serialization.h"
// #include "text/utf8char_serialization.h"

using namespace std::literals;

TEST_SUIT_BEGIN

// TEST_CASE("serialize utf8 char") {
//     {
//         auto c = Utf8Char{'3'};

//        auto json = nlohmann::json{};

//        json = c;

//        auto c2 = Utf8Char{json};

//        EXPECT_EQ(c, c2);
//    }
//}

// TEST_CASE("serialize fchar") {
//     {
//         auto c = FChar{'3', 10};

//        auto json = nlohmann::json{};

//        json = c;

//        auto c2 = FChar{json};

//        EXPECT_EQ(c.c, c2.c);
//        EXPECT_EQ(c.f, c2.f);
//    }
//}

// TEST_CASE("serialize fstring") {
//     {
//         auto c = FString{"hello there", 11};

//        auto json = nlohmann::json{};

//        json = c;

//        auto c2 = FString{json};

//        EXPECT_EQ(c, c2);
//        EXPECT_EQ(c.front().f, c2.front().f);
//    }
//}

TEST_SUIT_END
