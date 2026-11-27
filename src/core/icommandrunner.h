#pragma once

#include <filesystem>
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

/// A program and its arguments. No shell is involved unless the command is
/// created with Command::shell, so arguments (e.g. paths) need no quoting.
struct Command {
    std::vector<std::string> arguments;     // arguments[0] is the program
    std::filesystem::path workingDirectory; // Empty: inherit
    std::string input; // Written to stdin, which is then closed

    Command() = default;
    Command(std::vector<std::string> arguments,
            std::filesystem::path workingDirectory = {},
            std::string input = {})
        : arguments{std::move(arguments)}
        , workingDirectory{std::move(workingDirectory)}
        , input{std::move(input)} {}

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
    std::string output; // stdout, unless it has a callback
    std::string errors; // stderr, unless it has a callback
};

/// Where output goes while the program runs. A stream with a callback is
/// passed on in chunks, exactly as the program wrote it, and is not collected
/// in CommandResult. Callbacks run on the thread that runs the command.
struct CommandOutput {
    std::function<void(std::string_view)> output;
    std::function<void(std::string_view)> errors;
    /// Called once when both streams are closed.
    std::function<void()> finished;

    /// Write both streams to ostreams as they arrive.
    static CommandOutput to(std::ostream &output, std::ostream &errors);

    /// Deliver complete lines without the newline; a last line without a
    /// newline is delivered when the stream closes. Either may be empty, in
    /// which case that stream is collected instead.
    static CommandOutput lines(std::function<void(std::string_view)> output,
                               std::function<void(std::string_view)> errors);
};

/// A command running on its own thread, see ICommandRunner::start.
class RunningCommand {
public:
    RunningCommand() = default;
    RunningCommand(const RunningCommand &) = delete;
    RunningCommand(RunningCommand &&) = delete;
    RunningCommand &operator=(const RunningCommand &) = delete;
    RunningCommand &operator=(RunningCommand &&) = delete;

    /// Kills the program if it is still running, then waits for its thread.
    virtual ~RunningCommand() = default;

    /// Asks the program and everything it started to stop (SIGTERM).
    virtual void kill() = 0;

    /// Blocks until the program has exited and returns its result.
    virtual const CommandResult &wait() = 0;
};

class ICommandRunner {
public:
    using DoneT = std::function<void(const CommandResult &)>;

    ICommandRunner() = default;
    ICommandRunner(const ICommandRunner &) = delete;
    ICommandRunner(ICommandRunner &&) = delete;
    ICommandRunner &operator=(const ICommandRunner &) = delete;
    ICommandRunner &operator=(ICommandRunner &&) = delete;
    virtual ~ICommandRunner() = default;

    /// Runs on the calling thread and blocks until the program has exited.
    /// For short commands; prefer calling it from the job thread.
    virtual CommandResult run(const Command &,
                              const CommandOutput &stream = {}) = 0;

    /// Runs on a new thread, for long commands such as builds. `done` is
    /// called on that thread when the program has exited; post to a queue
    /// from it rather than touching editor state.
    virtual std::unique_ptr<RunningCommand> start(Command,
                                                  CommandOutput stream = {},
                                                  DoneT done = {}) = 0;
};

/// The runner for the current platform. Platforms without processes return a
/// runner whose commands always fail to start.
std::unique_ptr<ICommandRunner> createCommandRunner();
