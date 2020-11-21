#pragma once

#include "filesystem.h"
#include <vector>

class Project {

public:
    struct Settings {
        filesystem::path root;
        std::string buildCommand;
        std::vector<std::string> flags;
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

    //! When in a cpp-file find the corresponding .h-file. And vice versa
    //! @return path to file if found or empty path if not found
    filesystem::path findSwitchHeader(filesystem::path);

private:
    std::vector<filesystem::path> _fileCache;

    std::vector<filesystem::path> findProjectFiles(
        const filesystem::path &pathInProject, size_t max = 100000);

    void loadProjectFile();

    Settings _settings;
};