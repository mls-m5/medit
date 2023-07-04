#pragma once

#include "filesystem.h"
#include "meditfwd.h"
#include "text/rawbuffer.h"
#include <filesystem>
#include <string_view>

class IFile {
public:
    virtual void load(Buffer &) = 0;
    virtual void save(const Buffer &) = 0;
    virtual std::string representation() const = 0;
    virtual filesystem::path path() const = 0;

    virtual ~IFile() = default;

private:
    virtual void rename(std::filesystem::path to) = 0;

    friend class Files;
};
