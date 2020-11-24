
#define DO_NOT_CATCH_ERRORS

#include "files/file.h"
#include "files/project.h"
#include "mls-unit-test/unittest.h"
#include "mock/script/mockenvironment.h"
#include "text/buffer.h"
#include "text/cursorops.h"
#include "views/editor.h"
#include "clang/clangcompletion.h"
#include <memory>

const auto testPath1 = filesystem::path{"test/res/complete_test1.cpp"};
const auto testPath2 = filesystem::path{"test/res/complete_test2.cpp"};
constexpr auto testPosition1 = Position{11, 8};
constexpr auto testPosition2 = Position{6, 8};

TEST_SUIT_BEGIN

TEST_CASE("create") {
    auto completion = std::make_unique<ClangCompletion>();
}

TEST_CASE("simple complete") {
    auto completion = std::make_unique<ClangCompletion>();
    auto editor = Editor();
    auto env = MockEnvironment();
    auto project = Project{};
    env.mock_editor_0.returnValueRef(editor);
    env.mock_project_0.returnValueRef(project);

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

TEST_CASE("simple complete on unsaved file") {
    auto completion = std::make_unique<ClangCompletion>();
    auto editor = Editor();
    auto env = MockEnvironment();
    Project project;
    env.mock_editor_0.returnValueRef(editor);
    env.mock_project_0.returnValueRef(project);

    editor.file(std::make_unique<File>(testPath1));
    editor.load();
    editor.cursor({editor.buffer(), testPosition1});
    editor.cursor(insert('g', editor.cursor()));

    auto list = completion->list(env);

    ASSERT_GT(list.size(), 0);

    auto count = std::count_if(list.begin(), list.end(), [](auto &&item) {
        return item.name == "length";
    });

    ASSERT_GT(count, 0);
}

TEST_CASE("simple complete on unsaved file crossing dot") {
    auto completion = std::make_unique<ClangCompletion>();
    auto editor = Editor();
    auto env = MockEnvironment();
    Project project;
    env.mock_editor_0.returnValueRef(editor);
    env.mock_project_0.returnValueRef(project);

    editor.file(std::make_unique<File>(testPath2));
    editor.load();
    editor.cursor({editor.buffer(), {testPosition2.x(), testPosition2.y()}});

    for (auto c : "a.len") {
        editor.cursor(insert(c, editor.cursor()));
    }

    editor.cursor(left(editor.cursor()));

    std::cout << editor.buffer() << "\n";

    auto list = completion->list(env);

    ASSERT_GT(list.size(), 0);

    for (auto &item : list) {
        std::cout << item.name << " " << std::string{item.description} << "\n";
    }

    auto count = std::count_if(list.begin(), list.end(), [](auto &&item) {
        return item.name == "length";
    });

    ASSERT_GT(count, 0);
}

TEST_SUIT_END
