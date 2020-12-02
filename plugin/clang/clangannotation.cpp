#include "clang/clangannotation.h"
#include "core/context.h"
#include "core/jobqueue.h"
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

bool ClangAnnotation::annotate(std::shared_ptr<IEnvironment> env) {
    auto project = env->project();
    auto ss = std::ostringstream{};
    auto tmpFile = std::make_shared<TmpFile>(".cpp");
    ss << "clang++ -fsyntax-only -Wno-pragma-once-outside-header ";
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
                for (auto line : lines) {
                    if (!shouldShow) {
                        env->console().buffer().clear();
                    }
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

                env->context().redrawScreen();
            });
    });

    return true;
}

bool ClangAnnotation::shouldEnable(filesystem::path path) {
    return isCpp(path);
}

namespace {

struct Register {
    Register() {
        registerAnnotation<ClangAnnotation>();
    }
} r;

} // namespace
