#include "files/config.h"
#include "core/os.h"
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>

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

std::filesystem::path standardLocalFifoDirectory() {
    auto pidStr = std::filesystem::path{std::to_string(getPid())};
    if (auto path = localConfigDirectory("process" / pidStr / "fifo", true)) {
        return *path;
    }
    throw std::runtime_error{"could not create shared fifo path"};
}

/// Directory for where to put files for for example fifos used for the remote
/// fifo client and server
std::filesystem::path standardSharedFifoDirectory() {
    if (auto path = localConfigDirectory("fifos", true)) {
        return *path;
    }
    throw std::runtime_error{"could not create shared fifo path"};
}
std::filesystem::path fifoClientInPath() {
    return standardSharedFifoDirectory() / "fifo-client-in";
}

std::filesystem::path fifoClientOutPath() {
    return standardSharedFifoDirectory() / "fifo-client-out";
}

void cleanUpLocalPipes() {
    auto confPath = localConfigDirectory("process");
    if (!confPath) {
        return;
    }
    for (auto &it : std::filesystem::directory_iterator{*confPath}) {
        if (isProcessRunning(std::stol(it.path().filename().string()))) {
            continue;
        }
        std::filesystem::remove_all(it.path());
    }
}

std::filesystem::path standardConsoleTtyPipePath() {
    return standardLocalFifoDirectory() / "console-tty-in";
}

std::filesystem::__cxx11::path standardErrorTtyPipePath() {
    return standardLocalFifoDirectory() / "error-tty-in";
}
