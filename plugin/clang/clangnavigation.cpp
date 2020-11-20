#include "clang/clangnavigation.h"
#include "clangcontext.h"
#include "files/file.h"
#include "script/ienvironment.h"
#include "text/cursor.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include "clang/clangmodel.h"

using namespace std::literals;

void ClangNavigation::gotoSymbol(IEnvironment &env) {
    auto context = ClangContext{env, *getClangModel()};

    constexpr bool debug = true;

    if (debug) {
        env.console().buffer().clear();
        env.showConsole(true);
    }

    if (!context.translationUnit) {
        env.console().buffer().push_back(
            "goto definiton: failed to parse translation unit");
        return;
    }

    auto cursor = env.editor().cursor();

    auto cxCursor = context.getClangCursor(cursor);

    {
        auto usr = clang_getCursorUSR(cxCursor);
        auto str = clang_getCString(usr);
        if (str) {
            env.console().buffer().push_back("usr: "s + str);
        }
        clang_disposeString(usr);
    }
    {
        auto spelling = clang_getCursorSpelling(cxCursor);
        auto str = clang_getCString(spelling);
        if (str) {
            env.console().buffer().push_back("spelling: "s + str);
        }
        clang_disposeString(spelling);
    }
    {
        auto displayName = clang_getCursorDisplayName(cxCursor);
        auto str = clang_getCString(displayName);
        if (str) {
            env.console().buffer().push_back("displayName: "s + str);
        }
        clang_disposeString(displayName);
    }

    auto cxDefinition = clang_getCursorDefinition(cxCursor);

    if (clang_isInvalid(clang_getCursorKind(cxDefinition))) {
        env.showConsole(true);
        env.console().buffer().push_back(
            "goto definiton: failed to find declaration");
        return;
    }

    auto range = context.getRange(cxDefinition);

    auto location = context.getLocation(cxDefinition);
    auto defCursor = Cursor{env.editor().buffer(), location.position};

    //    auto cxFileNameString = clang_getFileName(range.file);
    auto cxFileNameString = clang_getFileName(location.file);

    auto cStringFilename = clang_getCString(cxFileNameString);
    auto filenameStr = std::string{cStringFilename ? cStringFilename : ""};

    clang_disposeString(cxFileNameString);

    if (debug) {

        env.console().buffer().push_back("definition:");
        auto tmp =
            content(CursorRange{env.editor().buffer(), range.begin, range.end});
        env.console().buffer().push_back(tmp.front());

        env.console().buffer().push_back(
            "line: "s + std::to_string(defCursor.y() + 1) + " " +
            std::to_string(defCursor.x() + 1));

        env.console().buffer().push_back(filenameStr);
    }

    auto path = env.editor().path();

    if (!filenameStr.empty() && path != filenameStr) {
        env.editor().file(std::make_unique<File>(filenameStr));
        env.editor().load();
    }

    env.editor().cursor({env.editor().buffer(), defCursor});

    env.showConsole(true);
}
