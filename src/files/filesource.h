#pragma once

#include "files/path.h"
#include <cstddef>
#include <map>
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
    /// The Git source could not use Git and listed the directory instead;
    /// the changed source found no repository.
    bool fellBack = false;
    /// Two-letter Git status ("XY" as in `git status --porcelain`) of each
    /// file, for the changed source; empty for the others.
    std::map<Path, std::string> status;

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

/// The files Git knows about: tracked and untracked, not ignored
/// (`git ls-files --cached --others --exclude-standard`). Tracked files missing
/// on disk are left out. Initialized submodules and nested repositories are
/// listed with their files. Outside a repository, or without git, it lists
/// the directory instead and sets `fellBack`.
std::unique_ptr<FileSource> createGitSource(ICommandRunner &runner,
                                            IFileSystem &fileSystem);

/// The files changed in Git (`git status --porcelain=v1 -z
/// --untracked-files=all`) below the root: modified, staged, untracked,
/// unmerged and deleted; a rename gives the new path. Initialized submodules
/// are listed with their own changed files. Sets `status` for each file.
/// Outside a repository the list is empty and `fellBack` is set.
std::unique_ptr<FileSource> createChangedSource(ICommandRunner &runner,
                                                IFileSystem &fileSystem);

/// Sorts and removes duplicates, and adds the settings file (when given),
/// which is always part of the project.
void finishProjectFiles(ProjectFiles &files, const Path &settingsFile);
