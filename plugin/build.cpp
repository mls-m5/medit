
#include "plugin/build.h"
#include "core/context.h"
#include "core/jobqueue.h"
#include "files/popenstream.h"
#include "files/project.h"
#include "syntax/ipalette.h"
#include "text/buffer.h"
#include "views/editor.h"

namespace {

bool isBuilding = false;

}

void build(std::shared_ptr<IEnvironment> env) {
    if (isBuilding) {
        return;
    }
    isBuilding = true;
    env->showConsole(true);
    auto &consoleBuffer = env->console().buffer();
    consoleBuffer.clear();
    consoleBuffer.push_back(std::string{"trying to build..."});

    auto &project = env->project();

    filesystem::current_path(project.settings().root);

    env->context().jobQueue().addTask([&project, &env] {
        POpenStream stream(project.settings().buildCommand, true, 100);

        for (std::string line; getline(stream, line);) {
            env->context().guiQueue().addTask([line, &env] {
                env->console().buffer().push_back(line);
                env->console().cursor({0, 100000000});
            });
        }

        env->context().guiQueue().addTask([returnCode = stream.returnCode(),
                                           &env] {
            auto &consoleBuffer = env->console().buffer();
            if (returnCode) {
                consoleBuffer.push_back(FString("failed...", IPalette::error));
            }
            else {
                consoleBuffer.push_back(std::string{"finished..."});
            }
        });

        isBuilding = false;
    });
}
