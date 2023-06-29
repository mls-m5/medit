
#include "project.h"
#include "core/files.h"
#include "files/directorynotifications.h"
#include "files/extensions.h"
#include "text/startswith.h"
#include "json/json.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string_view>

namespace {

const std::string_view projectFileName = ".medit.json";

std::string translateInclude(std::string flag, const filesystem::path &root) {
    if (starts_with(flag, "-I") && flag.size() > 2) {
        if (flag.at(2) != '/') {
            flag = "-I" + (root / flag.substr(2)).string();
        }
    }

    return flag;
}

} // namespace

Project::Project(DirectoryNotifications &directoryNotifications) {
    directoryNotifications.subscribe(
        [this](DirectoryNotifications::EventType type,
               std::filesystem::path path,
               std::filesystem::file_time_type) {
            if (type == DirectoryNotifications::Added) {
                addCachedFile(path);
            }
            else if (type == DirectoryNotifications::Removed) {
                removeCachedFile(path);
            }
        },
        this);
}

filesystem::path Project::root(filesystem::path path) const {
    path = filesystem::absolute(path);

    do {
        if (filesystem::exists(path / projectFileName)) {
            return path;
        }

        if (filesystem::exists(path / ".git")) {
            return path;
        }

        path = path.parent_path();

    } while (!path.empty());

    if (path.empty()) {
        return {};
    }

    return path.parent_path();
}

void Project::updateCache(const filesystem::path &pathInProject, size_t max) {
    _fileCache = findProjectFiles(pathInProject, max);
    loadProjectFile();
    if (_settings.buildCommand.empty()) {
        _settings.buildCommand = guessBuildCommand();
    }
}

filesystem::path Project::findSwitchHeader(filesystem::path path) {
    if (path.empty()) {
        return {};
    }
    auto extension = path.extension();
    if (!isCpp(path)) {
        return {};
    }

    bool isHeader = isCppHeader(path);

    auto stem = path.stem();

    for (auto &file : _fileCache) {
        if (file.stem() == stem) {
            if (file == path) {
                continue;
            }
            if (!isCpp(file)) {
                continue;
            }
            if (isHeader && isCppHeader(file)) {
                continue;
            }
            if (!isHeader && isCppSource(file)) {
                continue;
            }
            //! Todo: If more than one select the nearest one
            //            candidates.push_back(file);
            return file;
        }
    }

    return {};
}

std::string Project::guessBuildCommand() {
    if (std::filesystem::exists(_settings.root / "CMakeLists.txt")) {
        auto ss = std::ostringstream{};
        ss << "cd " << _settings.root << " ; "
           << " mkdir build/default ; "
           << "cd build/default ; "
              "cmake ../..  -DCMAKE_BUILD_TYPE=Debug "
              "-DCMAKE_EXPORT_COMPILE_COMMANDS=1 ;"
              " cmake --build .";
        return ss.str();
    }
    return {};
}

std::vector<filesystem::path> Project::findProjectFiles(
    const filesystem::path &pathInProject, size_t max) {

#ifdef __EMSCRIPTEN__

    return {};
#endif

    auto &root = _settings.root;
    root = this->root(pathInProject);

    if (root.empty()) {
        return {};
    }

    std::vector<filesystem::path> paths;

    size_t i = 0;
    for (auto &path : filesystem::recursive_directory_iterator{
             root,
             std::filesystem::directory_options::skip_permission_denied}) {
        if (!isKnownExtension(path)) {
            continue;
        }
        paths.push_back(path);
        ++i;
        if (i > max) {
            return paths;
        }
    }

    return paths;
}

void Project::loadProjectFile() {
    auto projectFile = _settings.root / projectFileName;
    if (!filesystem::exists(projectFile)) {
        return;
    }

    auto json = Json{};
    try {
        std::fstream(projectFile) >> json;
    }
    catch (Json::ParsingError &) {
    }

    if (auto it = json.find("build"); it != json.end()) {
        _settings.buildCommand = it->value;
    }

    if (auto it = json.find("run"); it != json.end()) {
        _settings.runCommand = it->value;
    }

    if (auto it = json.find("flags"); it != json.end()) {
        _settings.flags.clear();

        if (it->type == Json::String) {
            auto ss = std::istringstream();
            _settings.flags = std::vector<std::string>{};
            for (std::string s; ss >> s;) {
                _settings.flags.push_back(s);
            }
        }
        else if (it->type == Json::Array) {
            _settings.flags.clear();
            for (auto &value : *it) {
                if (value.type == Json::String) {
                    _settings.flags.push_back(
                        translateInclude(value.value, _settings.root));
                }
            }
        }
    }
}

void Project::addCachedFile(std::filesystem::path path) {
    if (std::find(_fileCache.begin(), _fileCache.end(), path) !=
        _fileCache.end()) {
        return;
    }
    _fileCache.push_back(path);
}

void Project::removeCachedFile(std::filesystem::path path) {
    _fileCache.erase(std::remove(_fileCache.begin(), _fileCache.end(), path));
}
