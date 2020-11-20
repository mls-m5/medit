
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
        auto file =
            clang_getFile(context.translationUnit, locationString.c_str());

        format(buffer, IPalette::standard);

        for (auto word : Words(buffer)) {
            auto location = clang_getLocation(context.translationUnit,
                                              file,
                                              word.begin().y() + 1,
                                              word.begin().x() + 1);
            auto ccursor = clang_getCursor(context.translationUnit, location);
            auto kind = clang_getCursorKind(ccursor);

            format(word, getFormat(kind));
        }
    }
}

void ClangHighlight::update(const IPalette &palette) {}
