
#include "project.h"
#include "core/ijobqueue.h"
#include "files/directorynotifications.h"
#include "files/extensions.h"
#include "files/projectsettings.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string_view>
#include <unordered_map>

Project::Project(std::filesystem::path projectPath,
                 DirectoryNotifications &directoryNotifications,
                 IJobQueue &guiQueue)
    : _tv{"Project"}
    , _guiQueue{&guiQueue} {
    directoryNotifications.subscribe(
        [this](DirectoryNotifications::EventType type,
               std::filesystem::path path,
               std::filesystem::file_time_type) {
            _guiQueue->addTask([this, path, type] {
                _tv();
                // #warning this is called on the wrong thread
                if (type == DirectoryNotifications::Added) {
                    addCachedFile(path);
                }
                else if (type == DirectoryNotifications::Removed) {
                    removeCachedFile(path);
                }
                else if (type == DirectoryNotifications::Changed) {
                    updateCachedFile(path);
                }
            });
        },
        this);

    auto projectFile = ProjectSettings::findProject(projectPath, true);
    _settings.load(projectFile);
}

void Project::updateCache(const std::filesystem::path &pathInProject,
                          size_t max) {
    _tv();
    _fileCache = findProjectFiles(pathInProject, max);
    loadProjectFile();
    if (_settings.buildCommand.empty()) {
        _settings.buildCommand = guessBuildCommand();
    }
}

std::filesystem::path Project::findSwitchHeader(std::filesystem::path path) {
    _tv();
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

Project::ProjectLanguage Project::getProjectLanguage() const {
    return guessProjectLanguage();
}

void Project::loadProjectFile() {
    _tv();

    auto projectFile = _settings.settingsPath;
    if (projectFile.empty()) {
        projectFile = _settings.root / ProjectSettings::projectFileName;
    }
    _settings.load(projectFile);
}

Project::ProjectLanguage Project::guessProjectLanguage() const {
    for (auto &ext : _extensions) {
        auto testFile = std::filesystem::path{"x"};
        testFile.replace_extension(ext.first);
        if (isCpp(testFile) || isC(testFile)) {
            return Cpp;
        }
        if (isGo(testFile)) {
            return Go;
        }
    }
    return Unknown;
}

std::string Project::guessBuildCommand() {
    _tv();
    if (std::filesystem::exists(_settings.root / "CMakeLists.txt")) {
        auto ss = std::ostringstream{};
        ss << "cd " << _settings.root << " ; "
           << " mkdir -p build/default ; "
           << "cd build/default ; "
              "cmake ../..  -DCMAKE_BUILD_TYPE=Debug "
              "-DCMAKE_EXPORT_COMPILE_COMMANDS=1 ;"
              " cmake --build .";
        return ss.str();
    }
    return {};
}

std::vector<std::filesystem::path> Project::findProjectFiles(
    const std::filesystem::path &pathInProject, size_t max) {
    _tv();

#ifdef __EMSCRIPTEN__

    return {};
#endif

    auto &root = _settings.root;
    // if (root.empty()) {
    //     root = this->findRoot(pathInProject);
    // }

    if (root.empty()) {
        return {};
    }

    std::vector<std::filesystem::path> paths;

    struct PathHash {
        std::size_t operator()(const std::filesystem::path &p) const {
            return std::hash<std::string>{}(p.string());
        }
    };

    auto extensions =
        std::unordered_map<std::filesystem::path, size_t, PathHash>{};

    size_t i = 0;
    for (auto it =
             std::filesystem::recursive_directory_iterator{
                 root,
                 std::filesystem::directory_options::skip_permission_denied};
         it != std::filesystem::recursive_directory_iterator{};
         ++it) {
        auto &path = it->path();

        if (std::filesystem::exists(path / "CMakeCache.txt")) {
            it.disable_recursion_pending();
            continue;
        }

        if (!isKnownExtension(path)) {
            continue;
        }
        if (!std::filesystem::is_regular_file(path)) {
            continue;
        }
        paths.push_back(path);
        if (path.has_extension()) {
            ++extensions[path.extension()];
        }
        ++i;
        if (i > max) {
            return paths;
        }
    }

    _extensions.clear();
    _extensions.assign(extensions.begin(), extensions.end());
    std::sort(_extensions.begin(), _extensions.end(), [](auto &a, auto &b) {
        return a.second > b.second;
    });

    return paths;
}

void Project::addCachedFile(std::filesystem::path path) {
    _tv();
    if (std::find(_fileCache.begin(), _fileCache.end(), path) !=
        _fileCache.end()) {
        return;
    }
    _fileCache.push_back(path);
    checkProjectFile(path);
}

void Project::removeCachedFile(std::filesystem::path path) {
    _tv();
    _fileCache.erase(std::remove(_fileCache.begin(), _fileCache.end(), path));
}

void Project::updateCachedFile(std::filesystem::path path) {
    _tv();
    checkProjectFile(path);
}

void Project::checkProjectFile(std::filesystem::path path) {
    if (std::filesystem::relative(path, _settings.root) != ".medit.json") {
        return;
    }
    loadProjectFile();
}
