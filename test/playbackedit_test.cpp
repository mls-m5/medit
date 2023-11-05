#include "codeplayback/edit.h"
#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "text/cursorops.h"
#include <memory>

TEST_SUIT_BEGIN(PlaybackEdit)

TEST_CASE("nested blocks") {
    auto text = R"(
struct Hello {
// block 1
you
// block n 2
there
// end n
// block multichar 3
howdy?
// end multichar
// end
};
)";
    auto buffer = Buffer{};

    auto edits = extractEditsFromString(buffer, text);

    EXPECT_EQ(edits.size(), 4);

    for (auto &edit : edits) {
        std::cout << "applying: " << edit.to << std::endl;
        apply(edit);
    }
}

TEST_SUIT_END
