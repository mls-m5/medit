#pragma once

#include "files/ifile.h"
#include <filesystem>

class File : public IFile {
    std::filesystem::path _path;

public:
    File(std::filesystem::path path);

    void load(Buffer &) override;
    void save(const Buffer &) override;
    std::string representation() const override;
    std::filesystem::path path() const override;

private:
    //! Function used by Files class to change the path
    void rename(std::filesystem::path to) override;

    friend class Files;
};
