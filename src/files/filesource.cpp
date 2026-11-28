#include "files/filesource.h"
#include "core/icommandrunner.h"
#include "files/extensions.h"
#include "files/ifilesystem.h"
#include <algorithm>
#include <string_view>

namespace {

class DirectorySource : public FileSource {
public:
    explicit DirectorySource(IFileSystem &fileSystem)
        : _fileSystem{fileSystem} {}

    std::string name() const override {
        return "directory";
    }

    ProjectFiles list(const Path &root, size_t limit) override {
        auto ret = ProjectFiles{root, {}, false};
        if (root.empty()) {
            return ret;
        }
        auto filter = ListFilter{};
        filter.includeFile = [](const Path &file) {
            return isKnownExtension(file);
        };
        filter.enterDirectory = [this](const Path &directory) {
            return directory.filename() != ".git" &&
                   !_fileSystem.stat(directory / "CMakeCache.txt").exists;
        };
        filter.limit = limit;
        ret.files = _fileSystem.list(root, filter);
        ret.truncated = ret.files.size() >= limit;
        return ret;
    }

private:
    IFileSystem &_fileSystem;
};

class GitSource : public FileSource {
public:
    GitSource(ICommandRunner &runner, IFileSystem &fileSystem)
        : _runner{runner}
        , _fileSystem{fileSystem}
        , _directory{fileSystem} {}

    std::string name() const override {
        return "git";
    }

    ProjectFiles list(const Path &root, size_t limit) override {
        auto ret = ProjectFiles{root, {}, false};
        if (root.empty()) {
            return ret;
        }
        if (!listRepository(root, limit, ret)) {
            ret = _directory.list(root, limit);
            ret.fellBack = true;
        }
        return ret;
    }

private:
    /// Adds the files of the repository at `repository` (the root or a
    /// submodule) to `files`. @returns false if git could not list it.
    bool listRepository(const Path &repository,
                        size_t limit,
                        ProjectFiles &files) {
        const auto result = _runner.run({{"git",
                                          "ls-files",
                                          "-z",
                                          "--cached",
                                          "--others",
                                          "--exclude-standard"},
                                         repository});
        if (result.exitCode != 0) {
            return false;
        }
        auto output = std::string_view{result.output};
        while (!output.empty()) {
            const auto end = output.find('\0');
            const auto name = output.substr(0, end);
            output = end == std::string_view::npos ? std::string_view{}
                                                   : output.substr(end + 1);
            if (name.empty()) {
                continue;
            }
            if (files.files.size() >= limit) {
                files.truncated = true;
                return true;
            }
            const auto path = repository / Path{std::string{name}};
            const auto info = _fileSystem.stat(path);
            if (!info.exists) {
                continue; // Deleted but still tracked.
            }
            if (info.isDirectory) {
                // A submodule or a nested repository; a link to a directory is
                // not followed.
                if (!info.isSymlink && _fileSystem.stat(path / ".git").exists) {
                    // Its files, or nothing if it cannot be listed.
                    listRepository(path, limit, files);
                }
                continue;
            }
            files.files.push_back(path);
        }
        return true;
    }

    ICommandRunner &_runner;
    IFileSystem &_fileSystem;
    DirectorySource _directory;
};

} // namespace

std::unique_ptr<FileSource> createGitSource(ICommandRunner &runner,
                                            IFileSystem &fileSystem) {
    return std::make_unique<GitSource>(runner, fileSystem);
}

std::unique_ptr<FileSource> createDirectorySource(IFileSystem &fileSystem) {
    return std::make_unique<DirectorySource>(fileSystem);
}

void finishProjectFiles(ProjectFiles &files, const Path &settingsFile) {
    for (auto &file : files.files) {
        file = file.lexically_normal();
    }
    if (!settingsFile.empty()) {
        files.files.push_back(settingsFile.lexically_normal());
    }
    std::sort(files.files.begin(), files.files.end());
    files.files.erase(std::unique(files.files.begin(), files.files.end()),
                      files.files.end());
}
