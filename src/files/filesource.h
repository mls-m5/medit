#pragma once

#include "files/path.h"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class IFileSystem;
class ICommandRunner;

/// The files of a project at one moment. Immutable once made.
struct ProjectFiles {
    Path root;
    std::vector<Path> files; // Absolute, sorted, unique
    bool truncated = false;  // The limit was reached; files are missing

    bool operator==(const ProjectFiles &) const = default;
};

/// Where the project's file list comes from (Git, the directory, ...).
class FileSource {
public:
    FileSource() = default;
    FileSource(const FileSource &) = delete;
    FileSource &operator=(const FileSource &) = delete;
    virtual ~FileSource() = default;

    virtual std::string name() const = 0;

    /// At most `limit` files below `root`. Runs on the job thread; must not
    /// touch editor state. The result does not need to be sorted.
    virtual ProjectFiles list(const Path &root, size_t limit) = 0;
};

/// Every regular file with a known extension below the root, skipping `.git`
/// and CMake build directories.
std::unique_ptr<FileSource> createDirectorySource(IFileSystem &fileSystem);

/// Sorts and removes duplicates, and adds the settings file (when given),
/// which is always part of the project.
void finishProjectFiles(ProjectFiles &files, const Path &settingsFile);
