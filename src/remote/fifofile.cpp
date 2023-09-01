#include "fifofile.h"
#include "files/config.h"

std::filesystem::path createFifo(std::filesystem::path path) {
    if (std::filesystem::exists(path)) {
        std::filesystem::remove(path);
    }

    if (std::system(("mkfifo \"" + path.string() + "\"").c_str())) {
        path.clear();
        throw std::runtime_error{"could not create fifo file " + path.string()};
    }

    return path;
}
