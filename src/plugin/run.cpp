#include "run.h"
#include "files/project.h"
#include "syntax/palette.h"
#include "text/buffer.h"
#include "views/console.h"
#include "views/consolenum.h"
#include "views/editor.h"

void run(std::shared_ptr<IEnvironment> env) {
#ifndef __EMSCRIPTEN__
    auto &project = env->project();

    auto root = project.settings().root;

    if (!root.empty()) {
        filesystem::current_path(root);
    }

    if (project.settings().runCommand.empty()) {
        auto &consoleBuffer = env->console().buffer();
        env->statusMessage(FString{"no run command specified..."});
        return;
    }

    auto runCommand = project.settings().runCommand;

    auto console = Console{
        .openingMessage =
            FString{"running application: ", Palette::comment} + runCommand,
        .command = runCommand,
        .buffer = &env->console().buffer(),
        .consoleNum = ConsoleNum::Run,
    };

    console.run(env);

#endif // __EMSCRIPTEN__
}
