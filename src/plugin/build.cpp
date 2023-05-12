
#include "plugin/build.h"
#include "files/project.h"
#include "script/ienvironment.h"
#include "syntax/palette.h"
#include "text/buffer.h"
#include "views/console.h"
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

    auto &consoleBuffer = env->console().buffer();
    consoleBuffer.pushBack(std::string{"trying to build..."});

    auto &project = env->project();

    auto root = project.settings().root;

    if (!root.empty()) {
        filesystem::current_path(root);
    }

    if (project.settings().buildCommand.empty()) {
        // TODO: Add guess with compile commands generation
        consoleBuffer.pushBack("no build command specified. Aborting");
        return;
    }

    auto command = project.settings().buildCommand;

    auto console = Console{
        .openingMessage =
            FString{"trying to build: ", Palette::comment} + command,
        .command = command,
        .buffer = &env->console().buffer(),
        .callback = [] { isBuilding = false; },
    };

    console.run(env);
#endif
}
