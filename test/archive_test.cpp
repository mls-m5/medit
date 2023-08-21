#include "core/archive.h"
#include "core/inarchive.h"
#include "core/outarchive.h"
#include "mls-unit-test/unittest.h"
#include "text/fstring.h"
#include "text/utf8char.h"
#include <sstream>

struct Apa {
    double x = 20;

    void visit(Archive &arch) {
        arch("x", x);
    }
};

TEST_SUIT_BEGIN

TEST_CASE("basic test") {
    auto ss = std::stringstream{};

    {
        auto arch = OutArchive{ss};

        int i = 10;
        EXPECT_FALSE(arch("hello", i));

        EXPECT_TRUE(arch.beginChild("child1"));

        i = 20;
        EXPECT_FALSE(arch("x", i));

        arch.endChild();

        auto v = std::vector<int>{1, 2, 3};
        EXPECT_FALSE(arch("list", v));

        auto c = Utf8Char{"ö"};

        EXPECT_FALSE(arch("char", c));

        //        auto fstr = FString{"hello", 2};
        //        EXPECT_FALSE(arch("fstr", fstr));
    }

    {
        auto arch = InArchive{ss};

        int i = 0;
        arch("hello", i);
        EXPECT_EQ(10, i);

        EXPECT_TRUE(arch.beginChild("child1"));

        EXPECT_TRUE(arch("x", i));

        EXPECT_EQ(i, 20);

        arch.endChild();

        auto v = std::vector<int>{1, 2, 3};
        EXPECT_TRUE(arch("list", v));

        EXPECT_EQ(v.size(), 3);
        EXPECT_EQ(v.at(0), 1);
        EXPECT_EQ(v.at(1), 2);
        EXPECT_EQ(v.at(2), 3);

        auto c = Utf8Char{};

        EXPECT_TRUE(arch("char", c));
        EXPECT_EQ(c, "ö");

        EXPECT_FALSE(arch.beginChild("non-existent"));
        size_t size = 10;
        EXPECT_FALSE(arch.beginList("non-existent", size));
    }
}

TEST_CASE("serialize visitable class") {

    auto ss = std::stringstream{};

    {
        auto apa = Apa{};
        apa.x = 22;
        auto arch = OutArchive{ss};
        arch("apa1", apa);
    }
    {
        auto apa = Apa{};
        auto arch = InArchive{ss};
        arch("apa1", apa);

        EXPECT_EQ(apa.x, 22);
    }
}

TEST_SUIT_END
