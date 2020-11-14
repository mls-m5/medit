#pragma once

#include "filesystem.h"
#include <vector>

class ProjectFiles {

public:
    struct Settings {
        filesystem::path root;
        std::string buildCommand;
        std::string formatCommand;
    };

    filesystem::path root(filesystem::path) const;

    void updateCache(const filesystem::path &pathInProject,
                     size_t max = 100000);

    const Settings &settings() const {
        return _settings;
    }

    const auto &files() {
        return _fileCache;
    }

private:
    std::vector<filesystem::path> _fileCache;

    std::vector<filesystem::path> findProjectFiles(
        const filesystem::path &pathInProject, size_t max = 100000);

    void loadProjectFile();

    Settings _settings;
};
