#pragma once

#include "files/ifile.h"
#include <filesystem>

class File : public IFile {
    std::filesystem::path _path;

public:
    File(std::filesystem::path path);

    void load(Buffer &) override;
    void save(const Buffer &) override;
};
