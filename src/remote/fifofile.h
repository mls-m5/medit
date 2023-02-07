#pragma once

#include <filesystem>

//! Create and remove fifo file
class FifoFile {
public:
    FifoFile(std::filesystem::path path);
    ~FifoFile();

    FifoFile(const FifoFile &) = delete;
    FifoFile &operator=(FifoFile &other) = delete;

    FifoFile(FifoFile &&other) {
        _path = std::move(other._path);
        other._path.clear();
    }

    FifoFile &operator=(FifoFile &&other) {
        _path = std::move(other._path);
        other._path.clear();
        return *this;
    }

    operator std::filesystem::path() const {
        return _path;
    }

    static FifoFile standardClientIn() {
        return FifoFile{clientInPath};
    }

    static FifoFile standardClientOut() {
        return FifoFile{clientOutPath};
    }

    const static std::filesystem::path clientInPath;
    const static std::filesystem::path clientOutPath;

private:
    std::filesystem::path _path;
};
