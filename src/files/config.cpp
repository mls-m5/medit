#include "files/config.h"

filesystem::path findConfig(filesystem::path file) {
    auto abs = filesystem::absolute(file);
    if (filesystem::exists(abs)) {
        return abs;
    }

    //! Handle if config is more difficult to locate
    return {};
}
