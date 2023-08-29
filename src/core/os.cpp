
#include "os.h"
#include <array>
#include <cstdlib>

namespace {

#ifdef MEDIT_USING_WINDOWS
filesystem::Path getPath() {
    return {}; // Not implemented
}

#else
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

#endif

} // namespace

std::filesystem::path executablePath() {
    return getPath();
}

int runCommand(const std::string &command) {
    return std::system(command.c_str());
}
