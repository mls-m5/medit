
#include "clang/clanghighlight.h"
#include "clangcontext.h"
#include "files/extensions.h"
#include "files/ifile.h"
#include "getformat.h"
#include "syntax/ipalette.h"
#include "text/buffer.h"
#include "text/cursorrangeops.h"
#include "text/words.h"
#include "views/editor.h"
#include "clang/clangmodel.h"

bool ClangHighlight::shouldEnable(filesystem::path path) {
    return isCpp(path);
}

void ClangHighlight::highlight(IEnvironment &env) {
    auto &editor = env.editor();
    if (!editor.file()) {
        return;
    }

    auto model = getClangModel();

    auto path = filesystem::absolute(editor.path());

    if (!isCpp(path)) {
        return;
    }

    auto locationString = path.string();

    auto &buffer = editor.buffer();

    auto context = ClangContext{env, *model};

    if (context.translationUnit) {
        if (true) {
            auto file =
                clang_getFile(context.translationUnit, context.tmpPath.c_str());

            format(buffer, IPalette::standard);

            for (auto word : Words(buffer)) {
                auto location = clang_getLocation(context.translationUnit,
                                                  file,
                                                  word.begin().y() + 1,
                                                  word.begin().x() + 1);
                auto ccursor =
                    clang_getCursor(context.translationUnit, location);
                auto kind = clang_getCursorKind(ccursor);

                format(word, getFormat(kind));
            }
        }

        else {
            // Alternative version that does not work as good yet
            // Probably because it also iterates over header files
            auto ccursor =
                clang_getTranslationUnitCursor(context.translationUnit);

            auto visitor = [&](CXCursor pos,
                               CXCursor /*parent*/) -> CXChildVisitResult {
                auto clangRange = context.getRange(pos);

                auto kind = clang_getCursorKind(pos);

                auto range =
                    CursorRange{buffer, clangRange.begin, clangRange.end};

                format(fix(range), getFormat(kind));

                return CXChildVisit_Recurse;
            };

            auto cvisitor =
                +[](CXCursor pos, CXCursor parent, CXClientData clientData) {
                    return (*static_cast<decltype(visitor) *>(clientData))(
                        pos, parent);
                };

            clang_visitChildren(
                ccursor, cvisitor, static_cast<CXClientData>(&visitor));
        }
    }
}

void ClangHighlight::update(const IPalette &palette) {}
