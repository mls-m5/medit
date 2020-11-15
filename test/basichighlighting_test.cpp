
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

TEST_CASE("format all to default") {
    BasicHighlighting hl;
    Editor editor;

    editor.buffer().text("hello"s);

    MockPalette palette;

    palette.mock_palette_const_0.returnValue(
        IPalette::BasicPalette{.standard = 4, .statement = 2});

    hl.update(palette);

    hl.highlight(editor);

    for (auto c : editor.buffer()) {
        ASSERT_EQ(c->f, 4);
    }
}

TEST_CASE("format keyword") {
    BasicHighlighting hl;
    Editor editor;

    editor.buffer().text("int hello"s);

    MockPalette palette;

    palette.mock_palette_const_0.returnValue(
        IPalette::BasicPalette{.standard = 1, .statement = 2});

    hl.update(palette);

    hl.highlight(editor);

    ASSERT_EQ(editor.buffer().front().f, 2);
    ASSERT_EQ(editor.buffer().back().f, 1);
}

TEST_CASE("partial match") {

    BasicHighlighting hl;
    Editor editor;

    editor.buffer().text("inte automatic"s);

    MockPalette palette;

    palette.mock_palette_const_0.returnValue(
        IPalette::BasicPalette{.standard = 1, .statement = 2});

    hl.update(palette);

    hl.highlight(editor);

    ASSERT_EQ(editor.buffer().front().f, 1);
    ASSERT_EQ(editor.buffer().back().f, 1);
}

TEST_SUIT_END