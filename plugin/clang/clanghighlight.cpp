
#include "clang/clanghighlight.h"
#include "files/extensions.h"
#include "files/ifile.h"
#include "getformat.h"
#include "syntax/ipalette.h"
#include "text/buffer.h"
#include "text/cursorrangeops.h"
#include "text/words.h"
#include "views/editor.h"
#include "clang/clangmodel.h"

namespace {

struct Range {
    Position begin, end;
    CXFile file;
};

Range getRange(CXCursor cursor) {
    Range range;
    auto extent = clang_getCursorExtent(cursor);

    unsigned int line, col;
    auto start = clang_getRangeStart(extent);
    clang_getSpellingLocation(start, &range.file, &line, &col, nullptr);
    range.begin = {col, line};

    auto end = clang_getRangeEnd(extent);
    clang_getSpellingLocation(end, nullptr, &line, &col, nullptr);
    range.end = {col, line};

    return range;
}

} // namespace

bool ClangHighlight::shouldEnable(filesystem::path path) {
    return isCpp(path);
}

void ClangHighlight::highlight(Editor &editor) {
    if (!editor.file()) {
        return;
    }
    auto model = getClangModel();

    auto path = filesystem::absolute(editor.file()->path());

    if (!isCpp(path)) {
        return;
    }

    auto locationString = path.string();

    auto &buffer = editor.buffer();

    const char *args[2] = {"-std=c++17", "-Iinclude"};

    auto translationUnit = clang_parseTranslationUnit(
        model->index, locationString.c_str(), args, 2, nullptr, 0, 0);

    auto file = clang_getFile(translationUnit, locationString.c_str());

    format(buffer, _palette.standard);

    for (auto word : Words(buffer)) {
        auto location = clang_getLocation(
            translationUnit, file, word.begin().y() + 1, word.begin().x() + 1);
        auto ccursor = clang_getCursor(translationUnit, location);
        CXCursorKind kind = clang_getCursorKind(ccursor);
        format(word, getFormat(kind, _palette));
    }
}

void ClangHighlight::update(const IPalette &palette) {
    _palette = palette.palette();
}
