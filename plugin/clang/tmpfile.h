// Copyright Mattias Larsson Sköld 2020
#pragma once

#include <experimental/filesystem>
#include <random>

namespace filesystem = std::experimental::filesystem;

//! Creates a path to a temporary file in /tmp folder on creation and removes
//! file when destructed
struct TmpFile {
    TmpFile() = delete;
    TmpFile(const TmpFile &) = delete;
    TmpFile &operator=(const TmpFile &) = delete;
    TmpFile(TmpFile &&other) {
        path = std::move(other.path);
        other.path.clear();
    }
    TmpFile &operator=(TmpFile &&other) {
        path = std::move(other.path);
        other.path.clear();
        return *this;
    }

    TmpFile(std::string extension, size_t len = 8) {
        std::random_device dev;
        std::mt19937 engine(dev());

        std::string filename;
        std::uniform_int_distribution<char> dist('a', 'z');

        for (size_t i = 0; i < len; ++i) {
            filename.push_back(dist(engine));
        }

        filename += extension;
        path = filesystem::path{"/tmp"} / filename;
    }

    ~TmpFile() {
        if (path.empty()) {
            return;
        }
        if (filesystem::exists(path)) {
            filesystem::remove(path);
        }
    }

    operator filesystem::path() {
        return path;
    }

    filesystem::path path;
};
