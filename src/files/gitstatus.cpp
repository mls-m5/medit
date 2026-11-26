#include "gitstatus.h"
#include "core/os.h"
#ifdef MEDIT_USING_LINUX
#include "files/path.h"
#include <array>
#include <cerrno>
#include <fcntl.h>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

std::set<std::filesystem::path> changedGitFiles(
    const std::filesystem::path &root) {
    std::set<std::filesystem::path> paths;
    if (root.empty())
        return paths;
    int pipefd[2];
    if (pipe(pipefd) < 0)
        return paths;
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&actions, pipefd[0]);
    posix_spawn_file_actions_addclose(&actions, pipefd[1]);
    posix_spawn_file_actions_addopen(
        &actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);
    std::string directory = root.string();
    char *args[] = {const_cast<char *>("git"),
                    const_cast<char *>("--no-optional-locks"),
                    const_cast<char *>("-c"),
                    const_cast<char *>("status.relativePaths=true"),
                    const_cast<char *>("-c"),
                    const_cast<char *>("core.fsmonitor=false"),
                    const_cast<char *>("-C"),
                    directory.data(),
                    const_cast<char *>("status"),
                    const_cast<char *>("--porcelain=v1"),
                    const_cast<char *>("-z"),
                    const_cast<char *>("--untracked-files=all"),
                    nullptr};
    pid_t child = -1;
    const int started =
        posix_spawnp(&child, "git", &actions, nullptr, args, environ);
    posix_spawn_file_actions_destroy(&actions);
    close(pipefd[1]);
    std::string output;
    bool readOk = true;
    if (started == 0) {
        std::array<char, 8192> buffer;
        for (;;) {
            const auto count = read(pipefd[0], buffer.data(), buffer.size());
            if (count > 0)
                output.append(buffer.data(), static_cast<size_t>(count));
            else if (count == 0)
                break;
            else if (errno != EINTR) {
                readOk = false;
                break;
            }
        }
    }
    close(pipefd[0]);
    if (started != 0)
        return paths;
    int status = 0;
    pid_t waited;
    do {
        waited = waitpid(child, &status, 0);
    } while (waited < 0 && errno == EINTR);
    if (!readOk || waited != child || !WIFEXITED(status) ||
        WEXITSTATUS(status) != 0)
        return paths;
    // -z places the destination first for renames/copies, followed by the
    // original path. Neither pathname is quoted, even if it contains spaces.
    for (size_t i = 0; i + 3 < output.size();) {
        const auto first = output[i], second = output[i + 1];
        if (output[i + 2] != ' ')
            break;
        i += 3;
        const auto end = output.find('\0', i);
        if (end == std::string::npos)
            break;
        paths.insert(normalizedAbsolutePath(root / output.substr(i, end - i)));
        i = end + 1;
        if (first == 'R' || first == 'C' || second == 'R' || second == 'C') {
            const auto originalEnd = output.find('\0', i);
            if (originalEnd == std::string::npos)
                break;
            i = originalEnd + 1;
        }
    }
    return paths;
}
#else
std::set<std::filesystem::path> changedGitFiles(const std::filesystem::path &) {
    return {};
}
#endif
