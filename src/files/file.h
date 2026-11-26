#pragma once

#include "files/ifile.h"
#include <cstdint>
#include <filesystem>
#include <optional>

class File : public IFile {
    std::filesystem::path _path;
    struct DiskState {
        std::uintmax_t size = 0;
        std::uintmax_t device = 0;
        std::uintmax_t inode = 0;
        std::int64_t modifiedSeconds = 0;
        std::int64_t modifiedNanoseconds = 0;
        std::int64_t changedSeconds = 0;
        std::int64_t changedNanoseconds = 0;
        std::filesystem::file_type type = std::filesystem::file_type::none;

        bool operator==(const DiskState &) const = default;
    };
    std::optional<DiskState> _diskState;
    bool _diskStateKnown = false;

    bool readDiskState(std::optional<DiskState> &) const;

public:
    File(std::filesystem::path path);

    void load(Buffer &) override;
    bool save(const Buffer &, bool overwriteExternal = false) override;
    bool hasExternalChanges() const override;
    std::string representation() const override;
    std::filesystem::path path() const override;

private:
    //! Function used by Files class to change the path
    void rename(std::filesystem::path to, bool externallyChanged) override;

    friend class Files;
};
