#include "core/archive.h"
#include "core/inarchive.h"
#include "core/outarchive.h"
#include "mls-unit-test/unittest.h"
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
        arch("hello", i);

        arch.beginChild("child1");

        i = 20;
        arch("x", i);

        arch.endChild();
    }

    {
        auto arch = InArchive{ss};

        int i = 0;
        arch("hello", i);
        EXPECT_EQ(10, i);

        arch.beginChild("child1");

        arch("x", i);

        EXPECT_EQ(i, 20);

        arch.endChild();
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
