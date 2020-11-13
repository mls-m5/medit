
#include "plugin/clangformat.h"
#include "files/extensions.h"
#include "views/editor.h"

void clangFormat(IEnvironment &env) {
    auto &e = env.editor();
    auto file = e.file();
    if (!file) {
        return;
    }

    auto path = file->path();

    if (!isCpp(path)) {
        return;
    }

    e.save();
    system(("clang-format " + std::string{path} + " -i").c_str());
    e.load();
}
