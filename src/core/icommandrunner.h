#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

/// A program and its arguments. No shell is involved unless the command is
/// created with Command::shell, so arguments (e.g. paths) need no quoting.
struct Command {
    std::vector<std::string> arguments;     // arguments[0] is the program
    std::filesystem::path workingDirectory; // Empty: inherit

    /// For commands written by the user, e.g. build commands in .medit.json,
    /// which are meant to be interpreted by the shell.
    static Command shell(std::string commandLine,
                         std::filesystem::path workingDirectory = {}) {
        return {{"/bin/sh", "-c", std::move(commandLine)},
                std::move(workingDirectory)};
    }

    /// Human readable form for titles and log messages. Not for execution.
    std::string description() const;
};

struct CommandResult {
    /// Exit status of the program. 128 + signal number if it was killed by a
    /// signal, -1 if it could not be started (see errors).
    int exitCode = -1;
    std::string output; // stdout, unless streamed
    std::string errors; // stderr, unless streamed
};

/// Optional line callbacks. A stream with a callback is delivered line by line
/// (without the newline) and is not collected in CommandResult.
struct CommandOutput {
    std::function<void(std::string_view)> output;
    std::function<void(std::string_view)> errors;
};

class ICommandRunner {
public:
    ICommandRunner() = default;
    ICommandRunner(const ICommandRunner &) = delete;
    ICommandRunner(ICommandRunner &&) = delete;
    ICommandRunner &operator=(const ICommandRunner &) = delete;
    ICommandRunner &operator=(ICommandRunner &&) = delete;
    virtual ~ICommandRunner() = default;

    /// Blocks until the program has exited; prefer calling it from the job
    /// thread. Callbacks run on the calling thread. Standard input is empty.
    virtual CommandResult run(const Command &,
                              const CommandOutput &stream = {}) = 0;
};

/// The runner for the current platform. Platforms without processes return a
/// runner whose commands always fail to start.
std::unique_ptr<ICommandRunner> createCommandRunner();
