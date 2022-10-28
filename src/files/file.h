#pragma once

#include "files/ifile.h"
#include "filesystem.h"
#include "text/rawbuffer.h"

class File : public IFile {
    filesystem::path _path;

public:
    File(filesystem::path path);

    void load(RawBuffer &) override;
    void save(const RawBuffer &) override;
    std::string representation() const override;
    filesystem::path path() const override;
};
