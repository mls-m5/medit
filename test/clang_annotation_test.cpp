
#include "files/file.h"
#include "files/filesystem.h"
#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "views/editor.h"
#include "clang/clanghighlight.h"

const filesystem::path testPath1 = "test/res/annotation_test1.cpp";

TEST_SUIT_BEGIN

TEST_CASE("basic annotation") {
    Editor editor;
    ClangHighlight highligt;

    editor.file(std::make_unique<File>(testPath1));

    highligt.highlight(editor);
}

TEST_SUIT_END
