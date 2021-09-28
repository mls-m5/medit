
#include "os.h"
#include <array>

namespace {

#ifdef MEDIT_USING_WINDOWS
filesystem::Path getPath() {
    return {}; // Not implemented
}

#else
filesystem::path getPath() {
    auto possibleLinks = std::array<filesystem::path, 3>{
        "/proc/self/exe",
        "/proc/curproc/file",
        "/proc/self/path/a.out",
    };

    for (auto &link : possibleLinks) {
        if (filesystem::exists(link) || filesystem::is_symlink(link)) {
            return filesystem::read_symlink(link);
        }
    }

    return {};
}

#endif

} // namespace

filesystem::path executablePath() {
    return getPath();
}
