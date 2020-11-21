#include "clang/clangannotation.h"
#include "files/extensions.h"
#include "files/popenstream.h"
#include "files/project.h"
#include "script/ienvironment.h"
#include "tmpfile.h"
#include "views/editor.h"
#include <fstream>
#include <sstream>

bool ClangAnnotation::annotate(IEnvironment &env) {
    auto project = env.project();
    std::ostringstream ss;
    TmpFile tmpFile{".cpp"};
    ss << "clang++ -fsyntax-only ";
    ss << tmpFile.path;
    for (auto &flag : project.settings().flags) {
        ss << " " << flag;
    }

    auto file = std::ofstream{tmpFile.path};
    file << env.editor().buffer().text();
    //    env.editor().buffer().text(file);

    POpenStream pstream(ss.str());

    // Todo parse message in future

    for (std::string line; getline(pstream, line);) {
        env.console().buffer().push_back(line);
    }

    env.showConsole(true);

    return true;
}

bool ClangAnnotation::shouldEnable(filesystem::path path) {
    return isCpp(path);
}
