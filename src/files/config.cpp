#include "files/config.h"
#include <cstdlib>

filesystem::path findConfig(filesystem::path file) {
    auto abs = filesystem::absolute(file);
    if (filesystem::exists(abs)) {
        return abs;
    }

    if (auto env = getenv("HOME")) {
        auto fullPath = filesystem::path{env} / ".config/medit" / file;
        if (filesystem::exists(fullPath)) {
            return fullPath;
        }
    }

    //! Handle if config is more difficult to locate
    return {};
}
