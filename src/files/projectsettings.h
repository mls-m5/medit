#pragma once

#include <filesystem>
#include <vector>

struct ProjectSettings {
    std::filesystem::path root;
    std::filesystem::path settingsPath;
    std::string buildCommand;
    std::string runCommand;
    std::vector<std::string> flags;
    std::string formatCommand;

    struct DebugInfo {
        std::string command;
        std::filesystem::path workingDir;
    };

    DebugInfo debug;

    bool load(std::filesystem::path path);
    void save();

    std::filesystem::path findProject(std::filesystem::path fileInPath);

    static std::vector<ProjectSettings> fetchRecentProjects();
    static void makeProjectAvailable(std::filesystem::path settingsPath);

    static inline const std::string_view projectFileName = ".medit.json";
    // When the project files are named
    static inline const std::string_view projectExtension = ".medit";
};
