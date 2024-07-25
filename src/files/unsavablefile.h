#pragma once

#include "files/ifile.h"
#include <filesystem>
#include <string>

class UnsavableFile : public IFile {
    int index = getIndex(); // Used to separate files with same name
    std::filesystem::path _path;

public:
    /// The path is only to sort and display it, not an actual path
    UnsavableFile(std::filesystem::path path)
        : _path{"tmp:/" + (path.string() + "(" + std::to_string(index) + ")")} {
    }

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

    int getIndex() {
        static int index = 0;
        return index++;
    }
};
