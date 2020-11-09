#pragma once

#include "filesystem.h"
#include <vector>

class ProjectFiles {
    std::vector<filesystem::path> _fileCache;

    std::vector<filesystem::path> findProjectFiles(
        const filesystem::path &pathInProject, size_t max = 100000) const;

public:
    filesystem::path root(filesystem::path) const;

    void updateCache(const filesystem::path &pathInProject,
                     size_t max = 100000);

    const auto &files() {
        return _fileCache;
    }
};
