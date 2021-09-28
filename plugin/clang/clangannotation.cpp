#include "clang/clangannotation.h"
#include "core/context.h"
#include "core/jobqueue.h"
#include "core/os.h"
#include "core/plugins.h"
#include "files/extensions.h"
#include "files/popenstream.h"
#include "files/project.h"
#include "script/ienvironment.h"
#include "text/startswith.h"
#include "tmpfile.h"
#include "views/editor.h"
#include <fstream>
#include <sstream>

namespace {

std::string getHighestClang() {
    if (getOs() == Os::Windows) {
        return "clang++.exe";
    }

    for (size_t i = 20; i > 2; --i) {
        auto command = "clang++-" + std::to_string(i);
        if (hasCommand(command)) {
            return command;
        }
    }

    return "clang++";
}

std::string getHighestGcc() {
    if (getOs() == Os::Windows) {
        return "g++.exe";
    }

    for (size_t i = 20; i > 2; --i) {
        auto command = "g++-" + std::to_string(i);
        if (hasCommand(command)) {
            return command;
        }
    }

    return "g++";
}

} // namespace

struct ClangAnnotation::Data {
    std::string _compiler;

    std::string compiler() {
        if (_compiler.empty()) {
            _compiler = getHighestClang();
            return _compiler;
        }

        if (_compiler.empty()) {
            _compiler = getHighestGcc();
            return _compiler;
        }

        return _compiler;
    }
};

ClangAnnotation::ClangAnnotation() : _data(std::make_unique<Data>()) {}

bool ClangAnnotation::annotate(std::shared_ptr<IEnvironment> env) {
    auto project = env->project();
    auto ss = std::ostringstream{};
    auto tmpFile = std::make_shared<TmpFile>(".cpp");
    ss << _data->compiler()
       << " -fsyntax-only -Wno-pragma-once-outside-header ";
    ss << tmpFile->path;
    for (auto &flag : project.settings().flags) {
        ss << " " << flag;
    }

    std::ofstream{tmpFile->path} << env->editor().buffer();

    auto command = ss.str();

    auto &jobQueue = env->context().jobQueue();

    jobQueue.addTask([env = std::move(env), command, tmpFile] {
        POpenStream pstream(command, true);

        auto pathStr = tmpFile->path.string();

        std::vector<std::string> lines;

        for (std::string line; getline(pstream, line);) {
            lines.push_back(std::move(line));
        }

        auto &guiQueue = env->context().guiQueue();

        guiQueue.addTask(
            [env = std::move(env), lines = std::move(lines), pathStr, command] {
                // Todo parse message in future
                bool shouldShow = false;
                env->console().buffer().clear();
                for (auto line : lines) {
                    if (starts_with(line, pathStr)) {
                        line.replace(0, pathStr.size(), env->editor().path());
                    }
                    env->console().buffer().push_back(line);
                    shouldShow = true;
                }

                if (shouldShow) {
                    env->showConsole(true);

                    env->console().buffer().push_back(" when compiling with");
                    env->console().buffer().push_back(command);
                }
                else {
                    env->console().buffer().push_back("no errors found");
                }

                env->context().redrawScreen();
            });
    });

    return true;
}

bool ClangAnnotation::shouldEnable(filesystem::path path) {
    return isCpp(path);
}

void ClangAnnotation::registerPlugin() {
    registerAnnotation<ClangAnnotation>();
}
