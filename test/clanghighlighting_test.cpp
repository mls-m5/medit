
#include "files/file.h"
#include "files/filesystem.h"
#include "files/project.h"
#include "mls-unit-test/unittest.h"
#include "mock/script/mockscope.h"
#include "text/buffer.h"
#include "views/editor.h"
#include "clang/clanghighlight.h"

const filesystem::path testPath1 = "test/res/annotation_test1.cpp";

TEST_SUIT_BEGIN

TEST_CASE("basic coloring unsaved file") {
    Editor editor;
    Project project;
    auto env = std::make_shared<MockScope>();
    env->mock_editor_0.returnValueRef(editor);
    env->mock_project_0.returnValueRef(project);
    ClangHighlight highligt;

    editor.file(std::make_unique<File>(testPath1));
    editor.load();

    highligt.highlight(env);
}

TEST_CASE("basic coloring") {
    Editor editor;
    Project project;
    auto env = std::make_shared<MockScope>();
    env->mock_editor_0.returnValueRef(editor);
    env->mock_project_0.returnValueRef(project);
    ClangHighlight highligt;

    editor.file(std::make_unique<File>(testPath1));

    highligt.highlight(env);
}

TEST_SUIT_END
