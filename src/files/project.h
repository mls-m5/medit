#pragma once

#include "meditfwd.h"
#include <filesystem>
#include <vector>

class Project {
public:
    Project(DirectoryNotifications &files);

    struct Settings {
        std::filesystem::path root;
        std::string buildCommand;
        std::string runCommand;
        std::vector<std::string> flags;
        std::string formatCommand;
    };

    /// Set the current root
    std::filesystem::path root(std::filesystem::path) const;

    void updateCache(const std::filesystem::path &pathInProject,
                     size_t max = 100000);

    const Settings &settings() const {
        return _settings;
    }

    const auto &files() {
        return _fileCache;
    }

    //! When in a cpp-file find the corresponding .h-file. And vice versa
    //! @return path to file if found or empty path if not found
    std::filesystem::path findSwitchHeader(std::filesystem::path);

private:
    /// When there is no build command provided. Try to guess what it could be
    /// based on what files are in the project
    std::string guessBuildCommand();

    std::vector<std::filesystem::path> _fileCache;

    std::vector<std::filesystem::path> findProjectFiles(
        const std::filesystem::path &pathInProject, size_t max = 100000);

    void loadProjectFile();

    Settings _settings;

    void addCachedFile(std::filesystem::path);
    void removeCachedFile(std::filesystem::path);
};
