
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

void build(std::shared_ptr<IEnvironment> env, std::function<void(bool)> then) {
#ifndef __EMSCRIPTEN__
    if (isBuilding) {
        env->statusMessage(FString{"already building..."});
        return;
    }
    isBuilding = true;

    auto &consoleBuffer = env->console().buffer();
    consoleBuffer.pushBack(std::string{"trying to build..."});

    auto &project = env->project();

    auto root = project.settings().root;

    if (!root.empty()) {
        std::filesystem::current_path(root);
    }

    if (project.settings().buildCommand.empty()) {
        // TODO: Add guess with compile commands generation
        consoleBuffer.pushBack("no build command specified. Aborting");
        isBuilding = false; // Otherwise no later build could start.
        if (then) {
            then(true); // Nothing to build.
        }
        return;
    }

    auto command = project.settings().buildCommand;

    auto console = Console{
        .openingMessage =
            FString{"trying to build: ", Palette::comment} + command,
        .command = Command::shell(command, root),
        .buffer = &env->console().buffer(),
        .callback =
            [then](int exitCode) {
                isBuilding = false;
                if (then) {
                    then(exitCode == 0);
                }
            },
    };

    console.run(env);
#endif
}
