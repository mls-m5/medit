
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

void build(std::shared_ptr<IScope> scope) {
#ifndef __EMSCRIPTEN__
    if (isBuilding) {
        return;
    }
    isBuilding = true;
    scope->env().showConsole(true);
    auto &consoleBuffer = scope->env().console().buffer();
    consoleBuffer.clear();
    consoleBuffer.pushBack(std::string{"trying to build..."});

    auto &project = scope->env().project();

    auto root = project.settings().root;

    if (!root.empty()) {
        filesystem::current_path(root);
    }

    scope->env().context().jobQueue().addTask([&project, scope] {
        POpenStream stream(project.settings().buildCommand, true, 100);

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
                        FString("failed...", IPalette::error));
                }
                else {
                    consoleBuffer.pushBack(std::string{"finished..."});
                }
            });

        isBuilding = false;
    });
#endif
}
