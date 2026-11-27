#include "core/icommandrunner.h"
#include <cstring>

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

#ifndef __EMSCRIPTEN__

#include <array>
#include <cerrno>
#include <fcntl.h>
#include <poll.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

namespace {

/// Collects one pipe and splits it into lines when a callback is given.
struct PipeReader {
    int fd = -1;
    const std::function<void(std::string_view)> *callback = nullptr;
    std::string *collected = nullptr;
    std::string pending;

    void append(std::string_view data) {
        if (!*callback) {
            collected->append(data);
            return;
        }
        pending.append(data);
        for (auto newline = pending.find('\n'); newline != std::string::npos;
             newline = pending.find('\n')) {
            (*callback)(std::string_view{pending}.substr(0, newline));
            pending.erase(0, newline + 1);
        }
    }

    void finish() {
        if (*callback && !pending.empty()) {
            (*callback)(pending);
            pending.clear();
        }
    }
};

void closeFd(int &fd) {
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

class PosixCommandRunner : public ICommandRunner {
public:
    CommandResult run(const Command &command,
                      const CommandOutput &stream) override {
        auto result = CommandResult{};
        if (command.arguments.empty()) {
            result.errors = "empty command";
            return result;
        }

        int outPipe[2] = {-1, -1};
        int errPipe[2] = {-1, -1};
        if (::pipe2(outPipe, O_CLOEXEC) || ::pipe2(errPipe, O_CLOEXEC)) {
            result.errors = std::string{"pipe: "} + std::strerror(errno);
            closeFd(outPipe[0]);
            closeFd(outPipe[1]);
            return result;
        }

        posix_spawn_file_actions_t actions;
        posix_spawn_file_actions_init(&actions);
        // Never let a child read from the terminal that ncurses owns.
        posix_spawn_file_actions_addopen(
            &actions, STDIN_FILENO, "/dev/null", O_RDONLY, 0);
        posix_spawn_file_actions_adddup2(&actions, outPipe[1], STDOUT_FILENO);
        posix_spawn_file_actions_adddup2(&actions, errPipe[1], STDERR_FILENO);
        if (!command.workingDirectory.empty()) {
            posix_spawn_file_actions_addchdir_np(
                &actions, command.workingDirectory.c_str());
        }

        auto argv = std::vector<char *>{};
        for (const auto &argument : command.arguments) {
            argv.push_back(const_cast<char *>(argument.c_str()));
        }
        argv.push_back(nullptr);

        pid_t pid = 0;
        const auto spawnError = posix_spawnp(
            &pid, argv.front(), &actions, nullptr, argv.data(), environ);
        posix_spawn_file_actions_destroy(&actions);
        closeFd(outPipe[1]);
        closeFd(errPipe[1]);

        if (spawnError) {
            closeFd(outPipe[0]);
            closeFd(errPipe[0]);
            result.errors = "could not start " + command.arguments.front() +
                            ": " + std::strerror(spawnError);
            return result;
        }

        auto readers = std::array<PipeReader, 2>{{
            {outPipe[0], &stream.output, &result.output, {}},
            {errPipe[0], &stream.errors, &result.errors, {}},
        }};

        // Read both pipes until they close, so a child that fills one of them
        // cannot block while we wait for the other.
        auto buffer = std::array<char, 4096>{};
        while (readers[0].fd >= 0 || readers[1].fd >= 0) {
            auto fds = std::array<pollfd, 2>{};
            for (size_t i = 0; i < readers.size(); ++i) {
                fds[i] = {readers[i].fd, POLLIN, 0};
            }
            if (::poll(fds.data(), fds.size(), -1) < 0) {
                if (errno == EINTR) {
                    continue;
                }
                break;
            }
            for (size_t i = 0; i < readers.size(); ++i) {
                if (readers[i].fd < 0 || !fds[i].revents) {
                    continue;
                }
                const auto count =
                    ::read(readers[i].fd, buffer.data(), buffer.size());
                if (count > 0) {
                    readers[i].append(
                        {buffer.data(), static_cast<size_t>(count)});
                }
                else if (count == 0 || errno != EINTR) {
                    closeFd(readers[i].fd);
                }
            }
        }
        for (auto &reader : readers) {
            closeFd(reader.fd);
            reader.finish();
        }

        int status = 0;
        while (::waitpid(pid, &status, 0) < 0) {
            if (errno != EINTR) {
                return result;
            }
        }
        if (WIFEXITED(status)) {
            result.exitCode = WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status)) {
            result.exitCode = 128 + WTERMSIG(status);
        }
        return result;
    }
};

} // namespace

std::unique_ptr<ICommandRunner> createCommandRunner() {
    return std::make_unique<PosixCommandRunner>();
}

#else

namespace {

class NullCommandRunner : public ICommandRunner {
public:
    CommandResult run(const Command &, const CommandOutput &) override {
        return {.exitCode = -1,
                .errors = "running commands is not supported here"};
    }
};

} // namespace

std::unique_ptr<ICommandRunner> createCommandRunner() {
    return std::make_unique<NullCommandRunner>();
}

#endif
