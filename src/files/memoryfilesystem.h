#pragma once

#include "files/ifilesystem.h"
#include <map>
#include <mutex>

/// A file system in memory, for tests. Directories exist implicitly when a
/// file is below them, or explicitly through `directory`.
class MemoryFileSystem : public IFileSystem {
public:
    std::optional<std::string> read(const Path &path) override;
    bool write(const Path &path, std::string_view content) override;
    bool rename(const Path &from, const Path &to) override;
    FileInfo stat(const Path &path) override;
    std::vector<Path> list(const Path &root, const ListFilter &filter) override;

    /// Adds an empty directory.
    void directory(const Path &path);
    void remove(const Path &path);

private:
    bool isDirectory(const Path &path) const;

    // Recursive: list() filters may call stat().
    mutable std::recursive_mutex _mutex;
    std::map<Path, std::string> _files;
    std::map<Path, bool> _directories;
};
