
#include "files/file.h"
#include "mls-unit-test/unittest.h"
#include "mock/script/mockienvironment.h"
#include "text/cursorops.h"
#include "views/editor.h"
#include "clang/clangcompletion.h"
#include <memory>

const auto testPath1 = filesystem::path{"test/res/complete_test1.cpp"};
constexpr auto testPosition1 = Position{11, 8};

TEST_SUIT_BEGIN

TEST_CASE("create") {
    auto completion = std::make_unique<ClangCompletion>();
}

TEST_CASE("simple complete") {
    auto completion = std::make_unique<ClangCompletion>();
    auto editor = Editor();
    auto env = MockIEnvironment();
    env.mock_editor_0.returnValueRef(editor);

    editor.file(std::make_unique<File>(testPath1));
    editor.load();
    editor.cursor({editor.buffer(), testPosition1});

    auto list = completion->list(env);

    ASSERT_GT(list.size(), 0);

    auto count = std::count_if(list.begin(), list.end(), [](auto &&item) {
        return item.name == "length";
    });

    ASSERT_GT(count, 0);
}

// TEST_CASE("simple complete on unsaved file") {
//    auto completion = std::make_unique<ClangCompletion>();
//    auto editor = Editor();
//    auto env = MockIEnvironment();
//    env.mock_editor_0.returnValueRef(editor);

//    editor.file(std::make_unique<File>(testPath1));
//    editor.load();
//    editor.cursor({editor.buffer(), testPosition1});
//    editor.cursor(insert('g', editor.cursor()));

//    auto list = completion->list(env);

//    ASSERT_GT(list.size(), 0);

//    auto count = std::count_if(list.begin(), list.end(), [](auto &&item) {
//        return item.name == "length";
//    });

//    ASSERT_GT(count, 0);
//}

TEST_SUIT_END
