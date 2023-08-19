#include "files/config.h"
#include "core/os.h"
#include <cstdlib>

namespace filesystem = std::filesystem;

filesystem::path findConfig(filesystem::path file) {
    if (auto env = getenv("HOME")) {
        auto fullPath = filesystem::path{env} / ".config/medit" / file;
        if (filesystem::exists(fullPath)) {
            return fullPath;
        }
    }

    if (auto exePath = executablePath(); !exePath.empty()) {
        auto fullPath = exePath.parent_path() / file;
        if (filesystem::exists(fullPath)) {
            return fullPath;
        }
    }

    auto abs = filesystem::absolute(file);
    if (filesystem::exists(abs)) {
        return abs;
    }

    //! Handle if config is more difficult to locate
    return {};
}
