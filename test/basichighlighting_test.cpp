
#include "mls-unit-test/unittest.h"
#include "mock/script/mockenvironment.h"
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
    auto env = std::make_shared<MockEnvironment>();
    env->mock_editor_0.returnValueRef(editor);

    editor.buffer().text("hello"s);

    MockPalette palette;

    hl.update(palette);

    hl.highlight(env);

    for (auto c : editor.buffer()) {
        ASSERT_EQ(c->f, 1);
    }
}

TEST_CASE("format keyword") {
    BasicHighlighting hl;
    Editor editor;
    auto env = std::make_shared<MockEnvironment>();
    env->mock_editor_0.returnValueRef(editor);

    editor.buffer().text("int hello"s);

    MockPalette palette;

    hl.update(palette);

    hl.highlight(env);

    ASSERT_EQ(editor.buffer().front().f, IPalette::statement);
    ASSERT_EQ(editor.buffer().back().f, 1);
}

TEST_CASE("partial match") {

    BasicHighlighting hl;
    Editor editor;
    auto env = std::make_shared<MockEnvironment>();
    env->mock_editor_0.returnValueRef(editor);

    editor.buffer().text("inte automatic"s);

    MockPalette palette;

    hl.update(palette);

    hl.highlight(env);

    ASSERT_EQ(editor.buffer().front().f, 1);
    ASSERT_EQ(editor.buffer().back().f, 1);
}

TEST_SUIT_END
