#pragma once

#include "core/ijobqueue.h"
#include "core/threadvalidation.h"
#include "meditfwd.h"
#include <filesystem>
#include <string>
#include <vector>

class Project {
public:
    enum ProjectLanguage {
        Cpp,
        Go,
        Unknown,
    };

    Project(DirectoryNotifications &files, IJobQueue &guiQueue);

    struct Settings {
        std::filesystem::path root;
        std::string buildCommand;
        std::string runCommand;
        std::vector<std::string> flags;
        std::string formatCommand;

        struct DebugInfo {
            std::string command;
            std::filesystem::path workingDir;
        };

        DebugInfo debug;
    };

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

    /// Get a list of most common used extensions sorted from most used to last
    /// Used to guess debugger and lsp
    const std::vector<std::pair<std::filesystem::path, size_t>> &extensions()
        const {
        return _extensions;
    }

    std::filesystem::path projectExtension() const {
        switch (getProjectLanguage()) {
        case Cpp:
            return ".cpp";
        case Go:
            return ".go";
        case Unknown:
            return {};
        }
        return {};
    }

    ProjectLanguage getProjectLanguage() const;

private:
    ProjectLanguage guessProjectLanguage() const;

    // Set the current root
    std::filesystem::path root(std::filesystem::path) const;

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

    std::vector<std::pair<std::filesystem::path, size_t>> _extensions;

    ThreadValidation _tv;

    IJobQueue *_guiQueue = nullptr;
};
