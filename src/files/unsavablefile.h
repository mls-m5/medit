#pragma once

#include "files/ifile.h"
#include <filesystem>

class UnsavableFile : public IFile {
    std::filesystem::path _path;

public:
    /// The path here is only
    UnsavableFile(std::filesystem::path path)
        : _path{path} {}

    void load(Buffer &) override {}
    bool save(const Buffer &) override {
        return false;
    }
    std::string representation() const override {
        return _path.string();
    }
    std::filesystem::path path() const override {
        return _path;
    }

private:
    //! Function used by Files class to change the path
    void rename(std::filesystem::path to) override {
        return;
    }
};
