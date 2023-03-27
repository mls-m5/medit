
#include "plugin/build.h"
#include "core/context.h"
#include "core/jobqueue.h"
#include "files/popenstream.h"
#include "files/project.h"
#include "script/ienvironment.h"
#include "syntax/palette.h"
#include "text/buffer.h"
#include "views/editor.h"

namespace {

bool isBuilding = false;
}

void build(std::shared_ptr<IEnvironment> env) {
#ifndef __EMSCRIPTEN__
    if (isBuilding) {
        return;
    }
    isBuilding = true;
    env->showConsole(true);
    auto &consoleBuffer = env->console().buffer();
    consoleBuffer.clear();
    consoleBuffer.pushBack(std::string{"trying to build..."});

    auto &project = env->project();

    auto root = project.settings().root;

    if (!root.empty()) {
        filesystem::current_path(root);
    }

    env->context().jobQueue().addTask([&project, env] {
        POpenStream stream(project.settings().buildCommand, true, 100);

        for (std::string line; getline(stream, line);) {
            env->context().guiQueue().addTask([line, env] {
                env->console().buffer().pushBack(line);
                env->console().cursor({0, 100000000});
            });
        }

        env->context().guiQueue().addTask([returnCode = stream.returnCode(),
                                           env] {
            auto &consoleBuffer = env->console().buffer();
            if (returnCode) {
                consoleBuffer.pushBack(FString("failed...", Palette::error));
            }
            else {
                consoleBuffer.pushBack(std::string{"finished..."});
            }
        });

        isBuilding = false;
    });
#endif
}
