#pragma once

#include <filesystem>
#include <vector>

struct ProjectSettings {
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

    void load(std::filesystem::path path);
};
