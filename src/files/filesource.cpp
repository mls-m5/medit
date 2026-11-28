#include "files/filesource.h"
#include "files/extensions.h"
#include "files/ifilesystem.h"
#include <algorithm>

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

} // namespace

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
