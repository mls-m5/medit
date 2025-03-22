#include "projectsettings.h"
#include "config.h"
#include "core/meditlog.h"
#include "text/startswith.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

const auto projectListFilename = std::filesystem::path{"projects.txt"};

std::filesystem::path projectDirectory() {
    if (auto p = localConfigDirectory("project", true)) {
        return *p;
    }
    else {
        return "";
    };
}

std::string translateInclude(std::string flag,
                             const std::filesystem::path &root) {
    if (starts_with(flag, "-I") && flag.size() > 2) {
        if (flag.at(2) != '/') {
            flag = "-I" + (root / flag.substr(2)).string();
        }
    }

    return flag;
}
} // namespace

bool ProjectSettings::load(std::filesystem::path projectFile) {
    if (!std::filesystem::exists(projectFile)) {
        return false;
    }

    settingsPath = projectFile;

    auto json = Json{};
    try {
        auto file = std::ifstream{projectFile};
        if (!file.is_open()) {
            throw std::runtime_error{"could not open file " +
                                     projectFile.string()};
        }
        file >> json;
    }
    catch (Json::ParsingError &error) {
        logStatusMessage(error.what());
        return false;
    }

    makeProjectAvailable(projectFile);

    if (auto it = json.find("root"); it != json.end()) {
        root = it->value;
    }
    else {
        if (projectFile.has_parent_path()) {
            root = std::filesystem::absolute(projectFile.parent_path());
        }
        else {
            root = findRoot(projectFile);
        }
    }

    // TODO: Add multiple build configuration
    if (auto it = json.find("build"); it != json.end()) {
        buildCommand = it->value;
    }

    // TODO: Add multiple build configuration
    if (auto it = json.find("run"); it != json.end()) {
        runCommand = it->value;
    }

    if (auto it = json.find("debug"); it != json.end()) {
        if (auto command = it->find("command"); command != it->end()) {
            debug.command = command->string();
        }
        if (auto dir = it->find("working_dir"); dir != it->end()) {
            debug.workingDir = dir->string();
        }
    }

    return true;
}

void ProjectSettings::save() {
    // TODO: Implement all of the settings

    auto json = Json{};

    if (settingsPath.empty()) {
        // Cannot save project
        return;
    }

    json["root"] = root;

    std::ofstream{settingsPath} << json;

    // TODO: Continue implementing this
}

std::filesystem::path ProjectSettings::findProject(
    std::filesystem::path fileInPath, bool createIfDoesNotExist) {
    auto projects = fetchRecentProjects();
    for (auto &project : projects) {
        if (fileInPath.parent_path().string().starts_with(
                project.settingsPath.string())) {
            return project.settingsPath;
        }
    }

    auto foundRoot = findRoot(fileInPath);
    if (foundRoot.empty()) {
        return {};
    }

    if (std::filesystem::exists(foundRoot / projectFileName)) {
        return foundRoot / projectFileName;
    }

    if (!createIfDoesNotExist) {
        return {};
    }
    return createAnonymousProject(foundRoot);
}

std::vector<ProjectSettings> ProjectSettings::fetchRecentProjects() {
    auto directory = projectDirectory();
    if (directory.empty()) {
        return {};
    }

    auto file = std::ifstream{directory / projectListFilename};
    auto list = std::vector<ProjectSettings>{};

    for (std::string line; std::getline(file, line);) {
        list.push_back({});
        auto &project = list.back();
        if (!project.load(line)) {
            list.pop_back();
        }
    }

    return list;
}

void ProjectSettings::makeProjectAvailable(std::filesystem::path path) {
    auto directory = projectDirectory();
    if (directory.empty()) {
        return;
    }

    auto list = std::vector<std::filesystem::path>{};

    {
        auto file = std::ifstream{directory / projectListFilename};

        for (std::string line; std::getline(file, line);) {
            auto path = std::filesystem::path{line};
            if (std::filesystem::exists(path)) {
                list.push_back(path);
            }
        }
    }

    if (std::find(list.begin(), list.end(), path) != list.end()) {
        // The project is already added
        return;
    }

    auto file = std::ofstream{directory / projectListFilename};

    list.insert(list.begin(), path);

    for (auto &item : list) {
        file << item.string() << "\n";
    }
}

std::filesystem::path ProjectSettings::findRoot(std::filesystem::path arg) {
    if (arg.empty()) {
        arg = std::filesystem::current_path();
    }
    auto path = std::filesystem::absolute(arg);

    while (!path.empty() && path != "/") {
        if (std::filesystem::exists(path / ProjectSettings::projectFileName)) {
            return path;
        }

        if (std::filesystem::exists(path / ".git")) {
            return path;
        }

        path = path.parent_path();
    }

    if (path.empty() || path == "/") {
        return {};
    }

    return arg.parent_path();
}

std::filesystem::path ProjectSettings::createAnonymousProject(
    std::filesystem::path root) {
    auto settings = ProjectSettings{};
    settings.settingsPath = (projectDirectory() / root.stem().string())
                                .replace_extension(projectExtension);
    settings.root = root;
    settings.save();
    settings.makeProjectAvailable(settings.settingsPath);
    return settings.settingsPath;
}
