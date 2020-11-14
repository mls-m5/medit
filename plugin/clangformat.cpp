
#include "plugin/clangformat.h"
#include "files/extensions.h"
#include "files/ifile.h"
#include "views/editor.h"

bool ClangFormat::format(Editor &editor) {
    auto file = editor.file();
    if (!file) {
        return false;
    }

    auto path = file->path();

    if (!isCpp(path)) {
        return false;
    }

    editor.save();
    system(("clang-format " + std::string{path} + " -i").c_str());
    editor.load();

    return true;
}
