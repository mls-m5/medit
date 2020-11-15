
#include "mls-unit-test/unittest.h"
#include "mock/syntax/mockpalette.h"
#include "syntax/basichighligting.h"
#include "syntax/palette.h"
#include "views/editor.h"

// Syntax test
// auto &operator ""_test(const char *val, size_t size) {
//    return unittest::testMap[std::string{val, size}];
//}
//"create"_test = []() {

//}

using namespace std::literals;

TEST_SUIT_BEGIN

TEST_CASE("create") {
    BasicHighlighting hl;
}

TEST_CASE("format all ") {
    BasicHighlighting hl;
    Editor editor;

    editor.buffer().text("hello"s);

    MockPalette palette;

    palette.mock_getFormat_const_1.returnValue(4);

    hl.update(palette);

    hl.highlight(editor);

    for (auto c : editor.buffer()) {
        ASSERT_EQ(c->f, 4);
    }
}

TEST_SUIT_END
