#pragma once

#include "meditfwd.h"
#include <filesystem>
#include <memory>
#include <mutex>
#include <vector>

/// Environment shared with all user of the program/server
class CoreEnvironment {
public:
    CoreEnvironment();

    CoreEnvironment(const CoreEnvironment &) = delete;
    CoreEnvironment(CoreEnvironment &&) = delete;
    CoreEnvironment &operator=(const CoreEnvironment &) = delete;
    CoreEnvironment &operator=(CoreEnvironment &&) = delete;

    std::shared_ptr<Buffer> open(std::filesystem::path);
    std::shared_ptr<Buffer> create();

    static CoreEnvironment &instance();

private:
    std::vector<std::shared_ptr<Buffer>> _buffers;
    std::mutex _fileMutex;
};
