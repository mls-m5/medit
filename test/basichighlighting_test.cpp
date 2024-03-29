
#include "files/file.h"
#include "mls-unit-test/unittest.h"
#include "mock/script/mockenvironment.h"
#include "syntax/basichighligting.h"
#include "syntax/palette.h"
#include "views/editor.h"
#include <memory>

// Syntax test
// auto &operator ""_test(const char *val, size_t size) {
//    return unittest::testMap[std::string{val, size}];
//}
//"create"_test = []() {

//}

using namespace std::literals;

TEST_SUIT_BEGIN(Basic_Highligting)

TEST_CASE("create") {
    BasicHighlighting hl;
}

TEST_CASE("format all to default") {
    BasicHighlighting hl;
    Editor editor{nullptr, std::make_shared<Buffer>()};
    auto env = std::make_shared<MockEnvironment>();
    env->mock_editor_0.returnValueRef(editor);

    editor.buffer().text("hello"s);

    hl.highlight(editor.buffer());

    for (auto c : editor.buffer()) {
        ASSERT_EQ(c.f, 1);
    }
}

TEST_CASE("format keyword") {
    BasicHighlighting hl;
    Editor editor{nullptr, std::make_shared<Buffer>()};
    auto env = std::make_shared<MockEnvironment>();
    env->mock_editor_0.returnValueRef(editor);

    editor.buffer().assignFile(std::make_unique<File>("test.cpp"));
    editor.buffer().text("int hello"s);

    hl.highlight(editor.buffer());

    ASSERT_EQ(editor.buffer().front().f, Palette::statement);
    ASSERT_EQ(editor.buffer().back().f, 1);
}

TEST_CASE("partial match") {

    BasicHighlighting hl;
    Editor editor{nullptr, std::make_shared<Buffer>()};
    auto env = std::make_shared<MockEnvironment>();
    env->mock_editor_0.returnValueRef(editor);

    editor.buffer().assignFile(std::make_unique<File>("test.cpp"));
    editor.buffer().text("inte automatic"s);

    hl.highlight(editor.buffer());

    ASSERT_EQ(editor.buffer().front().f, 1);
    ASSERT_EQ(editor.buffer().back().f, 1);
}

TEST_SUIT_END
