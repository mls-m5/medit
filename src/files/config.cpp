#include "files/config.h"
#include "core/os.h"
#include <cstdlib>

std::optional<std::filesystem::path> localConfigDirectory(
    std::filesystem::path relativePath, bool shouldCreate) {
    namespace filesystem = std::filesystem;
    if (auto env = getenv("HOME")) {
        auto home = filesystem::path{env};
        auto fullPath = home / ".config/medit" / relativePath;

        if (filesystem::is_directory(fullPath)) {
            return fullPath;
        }

        if (shouldCreate) {
            auto ec = std::error_code{};
            filesystem::create_directories(fullPath, ec);
            if (filesystem::is_directory(fullPath)) {
                return fullPath;
            }
        }
    }

    return std::nullopt;
}

std::filesystem::path findConfig(std::filesystem::path file) {
    namespace filesystem = std::filesystem;
    if (auto configPath = localConfigDirectory()) {
        auto fullPath = *configPath / file;
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

std::filesystem::path standardFifoDirectory() {
    if (auto path = localConfigDirectory("fifos", true)) {
        return *path;
    }
    throw std::runtime_error{"could not create fifo path"};
}
