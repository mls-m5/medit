
#include "project.h"
#include "core/ijobqueue.h"
#include "files/extensions.h"
#include "files/ifilesystem.h"
#include "files/projectsettings.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string_view>
#include <unordered_map>

Project::Project(std::filesystem::path projectPath,
                 IFileSystem &fileSystem,
                 IJobQueue &jobQueue,
                 IJobQueue &guiQueue)
    : _fileSystem{&fileSystem}
    , _source{createDirectorySource(fileSystem)}
    , _tv{"Project"}
    , _jobQueue{&jobQueue}
    , _guiQueue{&guiQueue} {
    auto projectFile = ProjectSettings::findProject(projectPath, true);
    _settings.load(projectFile);
}

Project::~Project() = default;

void Project::refreshNow() {
    _tv();
    loadProjectFile();
    auto files = _source->list(_settings.root, _fileLimit);
    finishProjectFiles(files, _settings.settingsPath);
    apply(std::move(files));
}

void Project::refresh() {
    _tv();
    if (_refreshing) {
        _refreshPending = true;
        return;
    }
    _refreshing = true;
    loadProjectFile();
    _jobQueue->addTask([source = _source,
                        root = _settings.root,
                        settingsPath = _settings.settingsPath,
                        limit = _fileLimit,
                        gui = _guiQueue,
                        alive = std::weak_ptr{_alive},
                        this] {
        auto files = source->list(root, limit);
        finishProjectFiles(files, settingsPath);
        gui->addTask([files = std::move(files), alive, this]() mutable {
            if (alive.expired()) {
                return;
            }
            _refreshing = false;
            apply(std::move(files));
            if (_refreshPending) {
                _refreshPending = false;
                refresh();
            }
        });
    });
}

std::shared_ptr<void> Project::subscribeToFiles(
    std::function<void()> callback) {
    _tv();
    auto handle = std::make_shared<std::function<void()>>(std::move(callback));
    _subscribers.push_back(handle);
    return handle;
}

void Project::apply(ProjectFiles files) {
    _tv();
    if (_settings.buildCommand.empty()) {
        _settings.buildCommand = guessBuildCommand();
    }
    if (files == _files) {
        return;
    }
    _files = std::move(files);
    updateExtensions();

    std::erase_if(_subscribers, [](auto &s) { return s.expired(); });
    // A copy: a callback may subscribe or unsubscribe.
    auto subscribers = _subscribers;
    for (auto &weak : subscribers) {
        if (auto callback = weak.lock()) {
            (*callback)();
        }
    }
}

void Project::updateExtensions() {
    struct PathHash {
        std::size_t operator()(const std::filesystem::path &p) const {
            return std::hash<std::string>{}(p.string());
        }
    };
    auto counts = std::unordered_map<std::filesystem::path, size_t, PathHash>{};
    for (auto &file : _files.files) {
        if (isKnownExtension(file) && file.has_extension()) {
            ++counts[file.extension()];
        }
    }
    _extensions.assign(counts.begin(), counts.end());
    std::sort(_extensions.begin(), _extensions.end(), [](auto &a, auto &b) {
        return a.second > b.second;
    });
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

    for (auto &file : _files.files) {
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
    if (_fileSystem->stat(_settings.root / "CMakeLists.txt").exists) {
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
