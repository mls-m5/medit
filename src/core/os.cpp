
#include "os.h"
#include <array>
#include <cstdlib>

#ifdef MEDIT_USING_WINDOWS

#include <windows.h>

filesystem::Path getPath() {
    return {}; // Not implemented
}

int getPid() {
    GetCurrentProcessId();
}

void setupSignals() {
    static_static_assert(false, "not implemented");
}

#else

#include <filesystem>
#include <signal.h>
#include <unistd.h>

std::filesystem::path getPath() {
    auto possibleLinks = std::array<std::filesystem::path, 3>{
        "/proc/self/exe",
        "/proc/curproc/file",
        "/proc/self/path/a.out",
    };

    for (auto &link : possibleLinks) {
        if (std::filesystem::exists(link) ||
            std::filesystem::is_symlink(link)) {
            return std::filesystem::read_symlink(link);
        }
    }

    return {};
}

int getPid() {
    return getpid();
}

bool isProcessRunning(int pid) {
    auto path = std::filesystem::path{"/proc/" + std::to_string(pid)};
    return std::filesystem::exists(path);
}

void setupSignals() {
    signal(SIGPIPE, SIG_IGN);
}

#endif

std::filesystem::path executablePath() {
    return getPath();
}

int runCommand(const std::string &command) {
    return std::system(command.c_str());
}
