#pragma once

#include "meditfwd.h"
#include <filesystem>
#include <string_view>

class IFile {
public:
    IFile() = default;
    IFile(const IFile &) = delete;
    IFile(IFile &&) = delete;
    IFile &operator=(const IFile &) = delete;
    IFile &operator=(IFile &&) = delete;

    virtual void load(Buffer &) = 0;

    /// Save buffer. @returns true on success
    virtual bool save(const Buffer &) = 0;
    virtual std::string representation() const = 0;
    virtual std::filesystem::path path() const = 0;

    virtual ~IFile() = default;

private:
    virtual void rename(std::filesystem::path to) = 0;

    friend class Files;
};
