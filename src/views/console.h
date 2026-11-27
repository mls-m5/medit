#pragma once

#include "core/icommandrunner.h"
#include "meditfwd.h"
#include "text/buffer.h"
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/// Just data to run commands in console (just a editor)
struct Console {
    FString openingMessage;
    Command command;
    Buffer *buffer = nullptr;
    /// Called on the gui thread with the exit code when the command finished.
    std::function<void(int exitCode)> callback;
    bool shouldShowConsole = true;
    bool shouldClear = true;

    /// Start a process on its own thread and print its output to the console
    void run(std::shared_ptr<IEnvironment> env);
};

/// The commands running in one window's console. Destroying it stops them.
/// Used from the gui thread only.
class ConsoleCommands {
public:
    /// Keeps a running command; returns an id for forget().
    size_t keep(std::unique_ptr<RunningCommand> command);

    /// Releases a command that has finished.
    void forget(size_t id);

    /// Asks every running command to stop (SIGTERM).
    void stopAll();

    size_t size() const {
        return _commands.size();
    }

private:
    size_t _nextId = 1;
    std::vector<std::pair<size_t, std::unique_ptr<RunningCommand>>> _commands;
};
