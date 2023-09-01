#pragma once

#include <filesystem>

//! Handles a specified file
//! Behaves like a unique_ptr. Removes file in owning destructor
class UniqueFile {
public:
    /// Create a fifo file with a specific name.
    UniqueFile(std::filesystem::path path)
        : _path{path} {}

    ~UniqueFile() {
        if (!_path.empty()) {
            std::filesystem::remove(_path);
        }
    }

    UniqueFile(const UniqueFile &) = delete;
    UniqueFile &operator=(UniqueFile &other) = delete;

    UniqueFile(UniqueFile &&other) {
        _path = std::move(other._path);
        other._path.clear();
    }

    UniqueFile &operator=(UniqueFile &&other) {
        _path = std::move(other._path);
        other._path.clear();
        return *this;
    }

    operator std::filesystem::path() const {
        return _path;
    }

    std::filesystem::path release() {
        return std::exchange(_path, {});
    }

private:
    std::filesystem::path _path;
};
