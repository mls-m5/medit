#include "clang/clangnavigation.h"
#include "files/file.h"
#include "script/ienvironment.h"
#include "text/cursor.h"
#include "views/editor.h"

void ClangNavigation::gotoSymbol(IEnvironment &env) {
    auto path = filesystem::absolute(env.editor().file()->path());
    auto cursor = env.editor().cursor();
}
