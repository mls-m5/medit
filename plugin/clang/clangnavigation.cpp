#include "clang/clangnavigation.h"
#include "clangcontext.h"
#include "files/file.h"
#include "script/ienvironment.h"
#include "text/cursor.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include "clang/clangmodel.h"

void ClangNavigation::gotoSymbol(IEnvironment &env) {
    auto context = ClangContext{env, *getClangModel()};

    if (!context.translationUnit) {
        return; // Failed
    }

    auto cursor = env.editor().cursor();

    auto cxCursor = context.getClangCursor(cursor);

    auto cxDefinition = clang_getCursorDefinition(cxCursor);

    auto range = context.getRange(cxDefinition);

    env.console().buffer().clear();
    env.console().buffer().push_back("definition:");
    auto tmp =
        content(CursorRange{env.editor().buffer(), range.begin, range.end});
    env.console().buffer().push_back(tmp.front());

    using namespace std::literals;

    env.console().buffer().push_back("line: "s +
                                     std::to_string(range.begin.y() + 1) + " " +
                                     std::to_string(range.begin.x() + 1));

    env.editor().cursor({env.editor().buffer(), range.begin});

    env.showConsole(true);
}
