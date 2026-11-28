#pragma once

#include "files/diskstate.h"
#include "meditfwd.h"
#include <filesystem>
#include <optional>
#include <string>
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

    /// Reads the file now and compares it with the last load or save. Files
    /// that are not on disk are always in sync.
    virtual DiskState diskState() const {
        return DiskState::InSync;
    }

    /// The content at the last load or save (the base for merging), or null
    /// when unknown or too large to keep.
    virtual const std::string *baseText() const {
        return nullptr;
    }

    /// The current content on disk, or nothing if it cannot be read.
    virtual std::optional<std::string> readDisk() const {
        return std::nullopt;
    }

    virtual ~IFile() = default;

private:
    virtual void rename(std::filesystem::path to) = 0;

    friend class Files;
};
