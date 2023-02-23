#include "run.h"
#include "core/context.h"
#include "core/jobqueue.h"
#include "files/popenstream.h"
#include "files/project.h"
#include "syntax/palette.h"
#include "text/buffer.h"
#include "views/editor.h"

void run(std::shared_ptr<IEnvironment> env) {
#ifndef __EMSCRIPTEN__

    env->showConsole(true);

    auto &project = env->project();

    auto root = project.settings().root;

    if (!root.empty()) {
        filesystem::current_path(root);
    }

    auto &consoleBuffer = env->console().buffer();
    consoleBuffer.clear();

    if (project.settings().runCommand.empty()) {
        consoleBuffer.pushBack(std::string{"no run command specified..."});
        return;
    }

    consoleBuffer.pushBack(std::string{"running application..."});

    env->context().jobQueue().addTask([&project, env] {
        POpenStream stream(project.settings().runCommand, true, 100);

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
    });

#endif // __EMSCRIPTEN__
}
