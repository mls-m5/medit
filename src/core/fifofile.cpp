#include "fifofile.h"
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sys/stat.h>

std::filesystem::path createFifo(std::filesystem::path path) {
    if (std::filesystem::exists(path)) {
        std::filesystem::remove(path);
    }

    // Only this user's processes write to these pipes.
    if (::mkfifo(path.c_str(), 0600) != 0) {
        throw std::runtime_error{"could not create fifo file " + path.string() +
                                 ": " + std::strerror(errno)};
    }

    return path;
}
