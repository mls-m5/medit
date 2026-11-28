#pragma once

#include "files/path.h"
#include <cstddef>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct FileInfo {
    bool exists = false;
    bool isDirectory = false;
    bool isSymlink = false; // The path itself is a link (to a file or not)
};

/// What IFileSystem::list returns.
struct ListFilter {
    /// Regular files to include; all of them if empty.
    std::function<bool(const Path &file)> includeFile;
    /// Directories to descend into; all of them if empty. Symlinks to
    /// directories are never followed.
    std::function<bool(const Path &directory)> enterDirectory;
    /// At most this many files are returned.
    size_t limit = std::numeric_limits<size_t>::max();
};

/// Everything medit does with files goes through this interface, so that it
/// can be replaced in tests and, later, by a remote file system. Watching is
/// separate (FileWatcher).
///
/// Implementations must be safe to call from several threads.
class IFileSystem {
public:
    IFileSystem() = default;
    IFileSystem(const IFileSystem &) = delete;
    IFileSystem &operator=(const IFileSystem &) = delete;
    virtual ~IFileSystem() = default;

    /// The content of a regular file, or nothing if missing or unreadable.
    virtual std::optional<std::string> read(const Path &) = 0;
    /// Replaces the file's content. @returns false on failure.
    virtual bool write(const Path &, std::string_view content) = 0;
    /// @returns false on failure; never replaces an existing `to`.
    virtual bool rename(const Path &from, const Path &to) = 0;
    virtual FileInfo stat(const Path &) = 0;
    /// The regular files below `root`, recursively, in no particular order.
    /// Unreadable directories are skipped.
    virtual std::vector<Path> list(const Path &root,
                                   const ListFilter &filter) = 0;
};

/// The local file system, shared by the whole process.
IFileSystem &localFileSystem();
