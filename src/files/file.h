#pragma once

#include "files/ifile.h"
#include "files/ifilesystem.h"
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>

/// The bytes `buffer` is saved as: its text, with a final newline added when
/// the last line is not empty.
std::string fileBytes(const Buffer &buffer);

class File : public IFile {
public:
    /// Larger files keep only a hash of their last loaded or saved content, so
    /// they are never merged.
    static constexpr size_t maxBaseSize = 4 * 1024 * 1024;

    File(std::filesystem::path path,
         IFileSystem &fileSystem = localFileSystem());

    void load(Buffer &) override;
    bool save(const Buffer &) override;
    std::string representation() const override;
    std::filesystem::path path() const override;
    DiskState diskState() const override;
    DiskState diskState(const DiskContent &) const override;
    void loaded(const std::string &bytes) override;
    size_t baselineVersion() const override {
        return _baselineVersion;
    }
    const std::string *baseText() const override;
    std::optional<std::string> readDisk() const override;
    DiskContent diskContent() const override;

private:
    /// What the file contained at the last load or save.
    struct Baseline {
        size_t hash = 0;
        std::optional<std::string> text; // Unless larger than maxBaseSize
    };

    void baseline(const std::string &bytes);

    //! Function used by Files class to change the path
    void rename(std::filesystem::path to) override;

    std::filesystem::path _path;
    IFileSystem *_fileSystem = nullptr;
    std::optional<Baseline> _baseline; // Nothing before the first load/save
    size_t _baselineVersion = 0;

    friend class Files;
};
