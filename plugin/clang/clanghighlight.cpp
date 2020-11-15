
#include "clang/clanghighlight.h"
#include "files/extensions.h"
#include "files/ifile.h"
#include "text/buffer.h"
#include "text/cursorrangeops.h"
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

void clangAnnotate(Editor &editor) {
    if (!editor.file()) {
        return;
    }
    auto model = getClangModel();

    auto path = filesystem::absolute(editor.file()->path());

    if (!isCpp(path)) {
        return;
    }

    auto locationString = path.string();

    auto buffer = editor.buffer();

    const char *args[2] = {"-std=c++17", "-Iinclude"};

    auto translationUnit = clang_parseTranslationUnit(
        model->index, locationString.c_str(), args, 2, nullptr, 0, 0);

    auto cursor = clang_getTranslationUnitCursor(translationUnit);

    auto file = clang_getFile(translationUnit, nullptr);

    auto visitor = [&editor, &file](CXCursor cursor,
                                    CXCursor /*parent*/) -> CXChildVisitResult {
        auto kind = clang_getCursorKind(cursor);
        auto range = getRange(cursor);

        if (!clang_File_isEqual(file, range.file)) {
            //        if (file != range.file) {
            return CXChildVisit_Recurse;
        }

        auto r = CursorRange{editor.buffer(), range.begin, range.end};

        format(r, 3);
        switch (kind) {
        case CXCursor_Namespace:
        case CXCursor_CXXMethod:
        case CXCursor_IfStmt:
            format(r, 3);

        default:
            break;
        }

        return CXChildVisit_Recurse;
    };

    auto cvisitor = +[](CXCursor cursor,
                        CXCursor parent,
                        CXClientData voidData) -> CXChildVisitResult {
        auto &data = *static_cast<decltype(visitor) *>(voidData);

        return data(cursor, parent);
    };

    clang_visitChildren(cursor, cvisitor, static_cast<void *>(&visitor));
}

} // namespace

bool ClangHighlight::shouldEnable(filesystem::path path) {
    return isCpp(path);
}

void ClangHighlight::highlight(Editor &editor) {
    clangAnnotate(editor);
}

void ClangHighlight::update(const IPalette &palette) {}
