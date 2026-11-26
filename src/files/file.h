#pragma once

#include "files/ifile.h"
#include <filesystem>
#include <optional>

class File : public IFile {
    std::filesystem::path _path;
    std::optional<std::filesystem::file_time_type> _diskTime;
    bool _diskTimeKnown = false;

    bool readDiskTime(std::optional<std::filesystem::file_time_type> &) const;

public:
    File(std::filesystem::path path);

    void load(Buffer &) override;
    bool save(const Buffer &, bool overwriteExternal = false) override;
    bool hasExternalChanges() const override;
    std::string representation() const override;
    std::filesystem::path path() const override;

private:
    //! Function used by Files class to change the path
    void rename(std::filesystem::path to) override;

    friend class Files;
};
