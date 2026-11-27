#include "console.h"
#include "core/context.h"
#include "core/coreenvironment.h"
#include "core/ijobqueue.h"
#include "script/ienvironment.h"
#include "script/staticcommandregister.h"
#include "syntax/palette.h"
#include "text/fstring.h"
#include "views/editor.h"
#include <algorithm>
#include <string>

namespace {

StaticCommandRegister consoleCommandsReg{{
    {"stop_console_commands",
     [](auto &&env) {
         env->consoleCommands().stopAll();
         env->statusMessage(FString{"stopping console commands..."});
     }},
}};

} // namespace

size_t ConsoleCommands::keep(std::unique_ptr<RunningCommand> command) {
    const auto id = _nextId++;
    _commands.emplace_back(id, std::move(command));
    return id;
}

void ConsoleCommands::forget(size_t id) {
    std::erase_if(_commands,
                  [id](const auto &command) { return command.first == id; });
}

void ConsoleCommands::stopAll() {
    for (auto &command : _commands) {
        command.second->kill();
    }
}

void Console::run(std::shared_ptr<IEnvironment> env) {
    env->showConsole(shouldShowConsole);

    auto &buffer = env->console().buffer();
    if (shouldClear) {
        buffer.clear();
    }
    buffer.pushBack(openingMessage);

    // The command runs on its own thread and must not keep the window alive:
    // all console updates go through the gui queue and check that the window
    // still exists. The queue outlives every window.
    auto weakEnv = std::weak_ptr{env};
    auto &queue = env->context().guiQueue();
    auto print = [weakEnv, &queue](FString line) {
        queue.addTask([weakEnv, line = std::move(line)] {
            if (auto env = weakEnv.lock()) {
                env->console().buffer().pushBack(line);
                env->console().cursor({0, 100000000});
            }
        });
    };

    auto output = CommandOutput::lines(
        [print](std::string_view line) { print(FString{std::string{line}}); },
        [print](std::string_view line) {
            print(FString{std::string{line}, Palette::error});
        });

    auto id = std::make_shared<size_t>(0);
    auto running = env->core().commandRunner().start(
        command,
        std::move(output),
        [weakEnv, &queue, id, callback = callback](
            const CommandResult &result) {
            queue.addTask([weakEnv, id, callback, result] {
                if (auto env = weakEnv.lock()) {
                    auto &consoleBuffer = env->console().buffer();
                    if (!result.errors.empty()) {
                        // Only set when the program could not be started.
                        consoleBuffer.pushBack(
                            FString{result.errors, Palette::error});
                    }
                    if (result.exitCode) {
                        consoleBuffer.pushBack(FString(
                            "failed (exit " + std::to_string(result.exitCode) +
                                ")...",
                            Palette::error));
                    }
                    else {
                        consoleBuffer.pushBack(
                            FString{"finished...", Palette::comment});
                    }
                    // Runs after keep(): this task is queued behind run().
                    env->consoleCommands().forget(*id);
                }
                if (callback) {
                    callback(result.exitCode);
                }
            });
        });
    *id = env->consoleCommands().keep(std::move(running));
}
