
#include "project.h"
#include "core/ijobqueue.h"
#include "core/meditlog.h"
#include "files/directorynotifications.h"
#include "files/extensions.h"
#include "text/startswith.h"
#include "json/json.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string_view>
#include <unordered_map>

namespace {

const std::string_view projectFileName = ".medit.json";

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

Project::Project(DirectoryNotifications &directoryNotifications,
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
}

std::filesystem::path Project::findRoot(std::filesystem::path arg) const {
    _tv();

    auto path = std::filesystem::absolute(arg);

    do {
        if (std::filesystem::exists(path / projectFileName)) {
            return path;
        }

        if (std::filesystem::exists(path / ".git")) {
            return path;
        }

        path = path.parent_path();

    } while (!path.empty() && path != "/");

    if (path.empty() || path == "/") {
        return {};
    }

    return arg.parent_path();
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
    root = this->findRoot(pathInProject);

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

void Project::loadProjectFile() {
    _tv();
    auto projectFile = _settings.root / projectFileName;
    if (!std::filesystem::exists(projectFile)) {
        return;
    }

    auto json = Json{};
    try {
        std::fstream(projectFile) >> json;
    }
    catch (Json::ParsingError &error) {
        logStatusMessage(error.what());
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

    if (auto it = json.find("debug"); it != json.end()) {
        if (auto command = it->find("command"); command != it->end()) {
            _settings.debug.command = command->string();
        }
        if (auto dir = it->find("working_dir"); dir != it->end()) {
            _settings.debug.workingDir = dir->string();
        }
    }
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
