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
    ss << "clang++ -fsyntax-only -Wno-pragma-once-outside-header ";
    ss << tmpFile.path;
    for (auto &flag : project.settings().flags) {
        ss << " " << flag;
    }

    std::ofstream{tmpFile.path} << env.editor().buffer();
    //    auto file = std::ofstream{tmpFile.path};
    //    file << env.editor().buffer().text();
    //    env.editor().buffer().text(file);

    auto command = ss.str();

    POpenStream pstream(command, true);

    // Todo parse message in future

    bool shouldShow = false;

    auto pathStr = tmpFile.path.string();

    for (std::string line; getline(pstream, line);) {
        if (!shouldShow) {
            env.console().buffer().clear();
        }
        if (line.starts_with(pathStr)) {
            line.replace(0, pathStr.size(), env.editor().path());
        }
        env.console().buffer().push_back(line);
        shouldShow = true;
    }

    if (shouldShow) {
        env.showConsole(true);

        env.console().buffer().push_back(" when compiling with");
        env.console().buffer().push_back(command);
    }

    //    // Test
    //    env.showConsole(true);
    //    env.console().buffer().push_back(command);

    return true;
}

bool ClangAnnotation::shouldEnable(filesystem::path path) {
    return isCpp(path);
}
