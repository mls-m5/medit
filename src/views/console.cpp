#include "console.h"
#include "core/context.h"
#include "core/ijobqueue.h"
#include "files/popenstream.h"
#include "script/ienvironment.h"
#include "syntax/palette.h"
#include "text/fstring.h"
#include "views/editor.h"

void Console::run(std::shared_ptr<IEnvironment> env) {
    env->showConsole(shouldShowConsole);

    auto &buffer = env->console().buffer();
    if (shouldClear) {
        buffer.clear();
    }

    env->context().jobQueue().addTask([command = this->command,
                                       env,
                                       callback = this->callback,
                                       openingMessage = openingMessage] {
        env->context().guiQueue().addTask([env, openingMessage] {
            env->console().buffer().pushBack(openingMessage);
        });
        POpenStream stream(command, true, 100);

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
                consoleBuffer.pushBack(
                    FString{"finished...", Palette::comment});
            }
        });

        if (callback) {
            callback();
        }
    });
}
