#include "core/icommandrunner.h"
#include <cstring>
#include <ostream>

std::string Command::description() const {
    auto ret = std::string{};
    for (const auto &argument : arguments) {
        if (!ret.empty()) {
            ret += ' ';
        }
        ret += argument;
    }
    return ret;
}

CommandOutput CommandOutput::to(std::ostream &output, std::ostream &errors) {
    return {
        .output =
            [&output](std::string_view chunk) {
                output.write(chunk.data(),
                             static_cast<std::streamsize>(chunk.size()));
            },
        .errors =
            [&errors](std::string_view chunk) {
                errors.write(chunk.data(),
                             static_cast<std::streamsize>(chunk.size()));
            },
        .finished =
            [&output, &errors] {
                output.flush();
                errors.flush();
            },
    };
}

namespace {

/// Splits chunks into lines. Shared by the copies of one CommandOutput.
struct LineSplitter {
    std::function<void(std::string_view)> callback;
    std::string pending;

    void append(std::string_view chunk) {
        pending.append(chunk);
        auto start = size_t{0};
        for (auto newline = pending.find('\n', start);
             newline != std::string::npos;
             newline = pending.find('\n', start)) {
            callback(std::string_view{pending}.substr(start, newline - start));
            start = newline + 1;
        }
        pending.erase(0, start);
    }

    void finish() {
        if (!pending.empty()) {
            callback(pending);
            pending.clear();
        }
    }
};

} // namespace

CommandOutput CommandOutput::lines(
    std::function<void(std::string_view)> output,
    std::function<void(std::string_view)> errors) {
    auto ret = CommandOutput{};
    auto out = std::shared_ptr<LineSplitter>{};
    auto err = std::shared_ptr<LineSplitter>{};
    if (output) {
        out = std::make_shared<LineSplitter>(std::move(output));
        ret.output = [out](std::string_view chunk) { out->append(chunk); };
    }
    if (errors) {
        err = std::make_shared<LineSplitter>(std::move(errors));
        ret.errors = [err](std::string_view chunk) { err->append(chunk); };
    }
    ret.finished = [out, err] {
        if (out) {
            out->finish();
        }
        if (err) {
            err->finish();
        }
    };
    return ret;
}

#ifndef __EMSCRIPTEN__

#include <array>
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <mutex>
#include <poll.h>
#include <pthread.h>
#include <spawn.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

extern char **environ;

namespace {

void closeFd(int &fd) {
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

/// The process of a command, shared with whoever may kill it. The pid is only
/// used for signals while the process is not yet reaped, so it cannot refer
/// to an unrelated process that reused the number.
struct ProcessHandle {
    std::mutex mutex;
    pid_t pid = 0;
    bool killRequested = false;

    void signal(int signal) {
        auto lock = std::lock_guard{mutex};
        killRequested = true;
        if (pid > 0) {
            ::kill(-pid, signal); // The whole process group.
        }
    }
};

/// Writing to a program that has stopped reading raises SIGPIPE. Block it on
/// this thread while the command runs and discard what it raised, so the
/// result does not depend on how the editor process handles the signal.
class BlockSigpipe {
public:
    BlockSigpipe() {
        sigemptyset(&_set);
        sigaddset(&_set, SIGPIPE);
        auto pending = sigset_t{};
        sigpending(&pending);
        _wasPending = sigismember(&pending, SIGPIPE);
        pthread_sigmask(SIG_BLOCK, &_set, &_previous);
    }

    ~BlockSigpipe() {
        if (!_wasPending) {
            auto zero = timespec{};
            while (sigtimedwait(&_set, nullptr, &zero) >= 0) {
            }
        }
        pthread_sigmask(SIG_SETMASK, &_previous, nullptr);
    }

    BlockSigpipe(const BlockSigpipe &) = delete;
    BlockSigpipe &operator=(const BlockSigpipe &) = delete;

private:
    sigset_t _set;
    sigset_t _previous;
    bool _wasPending = false;
};

struct Sink {
    int fd = -1;
    const std::function<void(std::string_view)> *callback = nullptr;
    std::string *collected = nullptr;

    void append(std::string_view chunk) const {
        if (*callback) {
            (*callback)(chunk);
        }
        else {
            collected->append(chunk);
        }
    }
};

CommandResult execute(const Command &command,
                      const CommandOutput &stream,
                      ProcessHandle &handle) {
    auto result = CommandResult{};
    auto notifyFinished = [&stream] {
        if (stream.finished) {
            stream.finished();
        }
    };

    if (command.arguments.empty()) {
        result.errors = "empty command";
        notifyFinished();
        return result;
    }

    auto blockSigpipe = BlockSigpipe{};

    int outPipe[2] = {-1, -1};
    int errPipe[2] = {-1, -1};
    int inPipe[2] = {-1, -1};
    const auto hasInput = !command.input.empty();
    if (::pipe2(outPipe, O_CLOEXEC) || ::pipe2(errPipe, O_CLOEXEC) ||
        (hasInput && ::pipe2(inPipe, O_CLOEXEC))) {
        result.errors = std::string{"pipe: "} + std::strerror(errno);
        for (auto *fd : {&outPipe[0],
                         &outPipe[1],
                         &errPipe[0],
                         &errPipe[1],
                         &inPipe[0],
                         &inPipe[1]}) {
            closeFd(*fd);
        }
        notifyFinished();
        return result;
    }

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    if (hasInput) {
        posix_spawn_file_actions_adddup2(&actions, inPipe[0], STDIN_FILENO);
    }
    else {
        // Never let a child read from the terminal that ncurses owns.
        posix_spawn_file_actions_addopen(
            &actions, STDIN_FILENO, "/dev/null", O_RDONLY, 0);
    }
    posix_spawn_file_actions_adddup2(&actions, outPipe[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, errPipe[1], STDERR_FILENO);
    if (!command.workingDirectory.empty()) {
        posix_spawn_file_actions_addchdir_np(&actions,
                                             command.workingDirectory.c_str());
    }

    // An own process group, so kill() also reaches what a shell starts. Reset
    // the signal mask and the signals the editor ignores, which a child would
    // otherwise inherit.
    posix_spawnattr_t attributes;
    posix_spawnattr_init(&attributes);
    posix_spawnattr_setflags(&attributes,
                             POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_SETSIGMASK |
                                 POSIX_SPAWN_SETSIGDEF);
    posix_spawnattr_setpgroup(&attributes, 0);
    auto emptyMask = sigset_t{};
    sigemptyset(&emptyMask);
    posix_spawnattr_setsigmask(&attributes, &emptyMask);
    auto defaults = sigset_t{};
    sigemptyset(&defaults);
    sigaddset(&defaults, SIGPIPE);
    sigaddset(&defaults, SIGINT);
    posix_spawnattr_setsigdefault(&attributes, &defaults);

    auto argv = std::vector<char *>{};
    for (const auto &argument : command.arguments) {
        argv.push_back(const_cast<char *>(argument.c_str()));
    }
    argv.push_back(nullptr);

    pid_t pid = 0;
    auto spawnError = 0;
    {
        auto lock = std::lock_guard{handle.mutex};
        spawnError = posix_spawnp(
            &pid, argv.front(), &actions, &attributes, argv.data(), environ);
        if (!spawnError) {
            handle.pid = pid;
            if (handle.killRequested) {
                ::kill(-pid, SIGTERM);
            }
        }
    }
    posix_spawn_file_actions_destroy(&actions);
    posix_spawnattr_destroy(&attributes);
    closeFd(outPipe[1]);
    closeFd(errPipe[1]);
    closeFd(inPipe[0]);

    if (spawnError) {
        closeFd(outPipe[0]);
        closeFd(errPipe[0]);
        closeFd(inPipe[1]);
        result.errors = "could not start " + command.arguments.front() + ": " +
                        std::strerror(spawnError);
        notifyFinished();
        return result;
    }

    if (inPipe[1] >= 0) {
        ::fcntl(inPipe[1], F_SETFL, ::fcntl(inPipe[1], F_GETFL) | O_NONBLOCK);
    }

    const auto sinks = std::array<Sink, 2>{{
        {outPipe[0], &stream.output, &result.output},
        {errPipe[0], &stream.errors, &result.errors},
    }};
    auto readFds = std::array<int, 2>{outPipe[0], errPipe[0]};
    auto inputFd = inPipe[1];
    auto written = size_t{0};

    // Serve all pipes at once, so a child that fills one of them cannot block
    // while we wait for another.
    auto buffer = std::array<char, 4096>{};
    while (readFds[0] >= 0 || readFds[1] >= 0 || inputFd >= 0) {
        auto fds = std::array<pollfd, 3>{{
            {readFds[0], POLLIN, 0},
            {readFds[1], POLLIN, 0},
            {inputFd, POLLOUT, 0},
        }};
        if (::poll(fds.data(), fds.size(), -1) < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        for (size_t i = 0; i < readFds.size(); ++i) {
            if (readFds[i] < 0 || !fds[i].revents) {
                continue;
            }
            const auto count = ::read(readFds[i], buffer.data(), buffer.size());
            if (count > 0) {
                sinks[i].append({buffer.data(), static_cast<size_t>(count)});
            }
            else if (count == 0 || errno != EINTR) {
                closeFd(readFds[i]);
            }
        }
        if (inputFd >= 0 && fds[2].revents) {
            const auto count = ::write(inputFd,
                                       command.input.data() + written,
                                       command.input.size() - written);
            if (count > 0) {
                written += static_cast<size_t>(count);
            }
            // The program may exit without reading everything (EPIPE).
            if (written == command.input.size() ||
                (count < 0 && errno != EAGAIN && errno != EINTR)) {
                closeFd(inputFd);
            }
        }
    }
    for (auto &fd : readFds) {
        closeFd(fd);
    }
    closeFd(inputFd);
    notifyFinished();

    // Wait without reaping, then reap while holding the lock, so a
    // concurrent kill() never signals a reused pid.
    auto info = siginfo_t{};
    while (::waitid(P_PID, pid, &info, WEXITED | WNOWAIT) < 0 &&
           errno == EINTR) {
    }
    int status = 0;
    {
        auto lock = std::lock_guard{handle.mutex};
        while (::waitpid(pid, &status, 0) < 0 && errno == EINTR) {
        }
        handle.pid = 0;
    }
    if (WIFEXITED(status)) {
        result.exitCode = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status)) {
        result.exitCode = 128 + WTERMSIG(status);
    }
    return result;
}

class PosixRunningCommand : public RunningCommand {
public:
    PosixRunningCommand(Command command,
                        CommandOutput stream,
                        ICommandRunner::DoneT done) {
        _thread = std::thread{[this,
                               command = std::move(command),
                               stream = std::move(stream),
                               done = std::move(done)] {
            _result = execute(command, stream, _handle);
            if (done) {
                // A copy: `done` may destroy this object.
                const auto result = _result;
                done(result);
            }
        }};
    }

    ~PosixRunningCommand() override {
        if (!_thread.joinable()) {
            return;
        }
        if (std::this_thread::get_id() == _thread.get_id()) {
            // Destroyed from its own `done` callback: the thread is finishing.
            _thread.detach();
            return;
        }
        _handle.signal(SIGKILL);
        _thread.join();
    }

    void kill() override {
        _handle.signal(SIGTERM);
    }

    const CommandResult &wait() override {
        if (_thread.joinable()) {
            _thread.join();
        }
        return _result;
    }

private:
    ProcessHandle _handle;
    CommandResult _result;
    std::thread _thread; // Last: started after the other members exist.
};

class PosixCommandRunner : public ICommandRunner {
public:
    CommandResult run(const Command &command,
                      const CommandOutput &stream) override {
        auto handle = ProcessHandle{};
        return execute(command, stream, handle);
    }

    std::unique_ptr<RunningCommand> start(Command command,
                                          CommandOutput stream,
                                          DoneT done) override {
        return std::make_unique<PosixRunningCommand>(
            std::move(command), std::move(stream), std::move(done));
    }
};

} // namespace

std::unique_ptr<ICommandRunner> createCommandRunner() {
    return std::make_unique<PosixCommandRunner>();
}

#else

namespace {

CommandResult notSupported(const CommandOutput &stream) {
    if (stream.finished) {
        stream.finished();
    }
    auto result = CommandResult{};
    result.errors = "running commands is not supported here";
    return result;
}

class FinishedCommand : public RunningCommand {
public:
    explicit FinishedCommand(CommandResult result)
        : _result{std::move(result)} {}

    void kill() override {}

    const CommandResult &wait() override {
        return _result;
    }

private:
    CommandResult _result;
};

class NullCommandRunner : public ICommandRunner {
public:
    CommandResult run(const Command &, const CommandOutput &stream) override {
        return notSupported(stream);
    }

    std::unique_ptr<RunningCommand> start(Command,
                                          CommandOutput stream,
                                          DoneT done) override {
        auto result = notSupported(stream);
        if (done) {
            done(result);
        }
        return std::make_unique<FinishedCommand>(std::move(result));
    }
};

} // namespace

std::unique_ptr<ICommandRunner> createCommandRunner() {
    return std::make_unique<NullCommandRunner>();
}

#endif
