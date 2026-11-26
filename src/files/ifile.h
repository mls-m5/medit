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
    virtual bool save(const Buffer &, bool overwriteExternal = false) = 0;
    // True when the backing file differs from the last observed version, or
    // when it cannot be checked reliably.
    virtual bool hasExternalChanges() const {
        return false;
    }
    virtual std::string representation() const = 0;
    virtual std::filesystem::path path() const = 0;

    virtual ~IFile() = default;

private:
    virtual void rename(std::filesystem::path to) = 0;

    friend class Files;
};
