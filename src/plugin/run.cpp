#include "run.h"
#include "core/context.h"
#include "core/jobqueue.h"
#include "files/popenstream.h"
#include "files/project.h"
#include "syntax/palette.h"
#include "text/buffer.h"
#include "views/editor.h"

void run(std::shared_ptr<IScope> scope) {
#ifndef __EMSCRIPTEN__

    scope->env().showConsole(true);

    auto &project = scope->env().project();

    auto root = project.settings().root;

    if (!root.empty()) {
        filesystem::current_path(root);
    }

    auto &consoleBuffer = scope->env().console().buffer();
    consoleBuffer.clear();

    if (project.settings().runCommand.empty()) {
        consoleBuffer.pushBack(std::string{"no run command specified..."});
        return;
    }

    consoleBuffer.pushBack(std::string{"running application..."});

    scope->env().context().jobQueue().addTask([&project, scope] {
        POpenStream stream(project.settings().runCommand, true, 100);

        for (std::string line; getline(stream, line);) {
            scope->env().context().guiQueue().addTask([line, scope] {
                scope->env().console().buffer().pushBack(line);
                scope->env().console().cursor({0, 100000000});
            });
        }

        scope->env().context().guiQueue().addTask(
            [returnCode = stream.returnCode(), scope] {
                auto &consoleBuffer = scope->env().console().buffer();
                if (returnCode) {
                    consoleBuffer.pushBack(
                        FString("failed...", Palette::error));
                }
                else {
                    consoleBuffer.pushBack(std::string{"finished..."});
                }
            });
    });

#endif // __EMSCRIPTEN__
}
