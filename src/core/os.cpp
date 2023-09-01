
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

#else

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

#endif

std::filesystem::path executablePath() {
    return getPath();
}

int runCommand(const std::string &command) {
    return std::system(command.c_str());
}
