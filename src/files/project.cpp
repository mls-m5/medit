
#include "project.h"
#include "core/ijobqueue.h"
#include "core/itimer.h"
#include "files/directorynotifications.h"
#include "files/extensions.h"
#include "files/gitstatus.h"
#include "files/path.h"
#include "files/projectsettings.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string_view>
#include <unordered_map>

namespace {

std::filesystem::path normalizedProjectPath(const std::filesystem::path &path) {
    return normalizedAbsolutePath(path);
}

bool isProjectBuildDirectory(const std::filesystem::path &path) {
    // A build/ subtree is excluded even before CMakeCache.txt exists.
    if (path.filename() == "build")
        return true;
    std::error_code error;
    return std::filesystem::exists(path / "CMakeCache.txt", error);
}

} // namespace

Project::Project(std::filesystem::path projectPath,
                 DirectoryNotifications &directoryNotifications,
                 IJobQueue &guiQueue,
                 IJobQueue *backgroundQueue,
                 ITimer *timer)
    : _tv{"Project"}
    , _directoryNotifications{&directoryNotifications}
    , _guiQueue{&guiQueue}
    , _backgroundQueue{backgroundQueue}
    , _timer{timer} {
    auto projectFile = ProjectSettings::findProject(projectPath, true);
    _settings.load(projectFile);

    directoryNotifications.subscribe(
        [this, lifetime = std::weak_ptr{_lifetime}, &guiQueue](
            DirectoryNotifications::EventType type,
            std::filesystem::path path,
            std::filesystem::file_time_type) {
            if (lifetime.expired()) {
                return;
            }
            guiQueue.addTask([this, lifetime, path, type] {
                if (lifetime.expired()) {
                    return;
                }
                _tv();
                requestGitStatusRefresh();
                std::error_code error;
                if (type == DirectoryNotifications::Rescan ||
                    (path.filename() == "CMakeCache.txt" &&
                     type != DirectoryNotifications::Changed) ||
                    (type == DirectoryNotifications::Added &&
                     std::filesystem::is_directory(path, error))) {
                    updateCache();
                }
                else if (type == DirectoryNotifications::Added) {
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

Project::~Project() {
    _tv();
    _lifetime.reset();
    if (_timer)
        _timer->cancel(_gitStatusTimer);
    _directoryNotifications->unsubcribe(this);
    for (auto &subscriber : _filesSubscribers)
        subscriber->active = false;
    for (auto &subscriber : _gitSubscribers)
        subscriber->active = false;
}

void Project::enableGitStatus() {
    _tv();
    if (_gitEnabled)
        return;
    _gitEnabled = true;
    requestGitStatusRefresh();
}

void Project::requestGitStatusRefresh() {
    _tv();
    if (!_gitEnabled || !_timer || !_backgroundQueue)
        return;
    _timer->cancel(_gitStatusTimer);
    const auto generation = ++_gitGeneration;
    const auto root = normalizedProjectPath(_settings.root);
    auto lifetime = std::weak_ptr{_lifetime};
    auto *background = _backgroundQueue;
    auto *gui = _guiQueue;
    _gitStatusTimer = _timer->setTimeout(
        std::chrono::milliseconds{200},
        [this, generation, root, lifetime, background, gui] {
            background->addTask([this, generation, root, lifetime, gui] {
                if (lifetime.expired())
                    return;
                auto changed = changedGitFiles(root);
                gui->addTask([this,
                              generation,
                              root,
                              lifetime,
                              changed = std::move(changed)]() mutable {
                    if (lifetime.expired() || generation != _gitGeneration ||
                        root != normalizedProjectPath(_settings.root))
                        return;
                    if (_gitChanged != changed) {
                        _gitChanged = std::move(changed);
                        emitGitStatusChanged();
                    }
                });
            });
        });
}

bool Project::gitChanged(const std::filesystem::path &path) const {
    _tv();
    // Tree paths and cached paths are already normalized; no filesystem work
    // or Git invocation belongs in the drawing path.
    return _gitChanged.contains(path);
}

void Project::subscribeToGitStatusChanged(FilesChangedCallbackT callback,
                                          void *ref) {
    _tv();
    _gitSubscribers.push_back(std::make_shared<FilesSubscriber>(
        FilesSubscriber{std::move(callback), ref}));
}

void Project::unsubscribeFromGitStatusChanged(void *ref) {
    _tv();
    std::erase_if(_gitSubscribers, [ref](const auto &subscriber) {
        if (subscriber->ref != ref)
            return false;
        subscriber->active = false;
        return true;
    });
}

void Project::emitGitStatusChanged() {
    _tv();
    const auto subscribers = _gitSubscribers;
    for (const auto &subscriber : subscribers)
        if (subscriber->active && subscriber->callback)
            subscriber->callback();
}

void Project::subscribeToFilesChanged(FilesChangedCallbackT callback,
                                      void *ref) {
    _tv();
    _filesSubscribers.push_back(std::make_shared<FilesSubscriber>(
        FilesSubscriber{std::move(callback), ref}));
}

void Project::unsubscribeFromFilesChanged(void *ref) {
    _tv();
    std::erase_if(_filesSubscribers, [ref](auto &subscriber) {
        if (subscriber->ref != ref) {
            return false;
        }
        subscriber->active = false;
        return true;
    });
}

void Project::emitFilesChanged() {
    _tv();
    // A callback may unsubscribe itself/another callback or add a subscriber.
    // New subscribers start with the next change; removed ones are skipped.
    const auto subscribers = _filesSubscribers;
    for (const auto &subscriber : subscribers) {
        if (subscriber->active && subscriber->callback) {
            subscriber->callback();
        }
    }
}

void Project::updateCache(size_t max) {
    _tv();
    loadProjectFile();
    const auto root = normalizedProjectPath(_settings.root);
    if (root != _watchedRoot) {
        _directoryNotifications->path(root);
        _watchedRoot = root;
        if (!_gitChanged.empty()) {
            _gitChanged.clear();
            emitGitStatusChanged();
        }
    }
    requestGitStatusRefresh();
    auto paths = findProjectFiles(root, max);
    if (_settings.buildCommand.empty()) {
        _settings.buildCommand = guessBuildCommand();
    }
    if (paths != _fileCache) {
        _fileCache = std::move(paths);
        updateExtensions();
        emitFilesChanged();
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

bool Project::shouldIncludeFile(const std::filesystem::path &path) const {
    std::error_code error;
    if (path.empty() || !std::filesystem::is_regular_file(path, error)) {
        return false;
    }
    // Anonymous project settings may live outside the project root.
    if (path == normalizedProjectPath(_settings.settingsPath)) {
        return true;
    }
    const auto root = normalizedProjectPath(_settings.root);
    const auto relative = path.lexically_relative(root);
    if (root.empty() || relative.empty() || relative.is_absolute() ||
        *relative.begin() == ".." || !isKnownExtension(path)) {
        return false;
    }
    for (auto parent = path.parent_path(); parent != root;
         parent = parent.parent_path()) {
        if (isProjectBuildDirectory(parent)) {
            return false;
        }
    }
    return true;
}

void Project::updateExtensions() {
    auto counts = std::unordered_map<std::filesystem::path, size_t>{};
    for (const auto &path : _fileCache) {
        if (path.has_extension()) {
            ++counts[path.extension()];
        }
    }
    _extensions.assign(counts.begin(), counts.end());
    std::sort(_extensions.begin(), _extensions.end(), [](auto &a, auto &b) {
        return a.second != b.second ? a.second > b.second : a.first < b.first;
    });
}

std::vector<std::filesystem::path> Project::findProjectFiles(
    const std::filesystem::path &pathInProject, size_t max) {
    _tv();

#ifdef __EMSCRIPTEN__

    return {};
#endif

    const auto root = normalizedProjectPath(pathInProject);

    if (root.empty()) {
        return {};
    }

    std::vector<std::filesystem::path> paths;

    // Directories may disappear between a kernel event and GUI delivery.
    std::error_code error;
    for (auto it =
             std::filesystem::recursive_directory_iterator{
                 root,
                 std::filesystem::directory_options::skip_permission_denied,
                 error};
         !error && it != std::filesystem::recursive_directory_iterator{} &&
         paths.size() < max;
         it.increment(error)) {
        const auto path = normalizedProjectPath(it->path());

        if (isProjectBuildDirectory(path)) {
            it.disable_recursion_pending();
            continue;
        }

        if (shouldIncludeFile(path)) {
            paths.push_back(path);
        }
    }

    const auto settingsPath = normalizedProjectPath(_settings.settingsPath);
    if (shouldIncludeFile(settingsPath)) {
        paths.push_back(settingsPath);
    }
    std::sort(paths.begin(), paths.end());
    paths.erase(std::unique(paths.begin(), paths.end()), paths.end());

    return paths;
}

void Project::addCachedFile(std::filesystem::path path) {
    _tv();
    path = normalizedProjectPath(path);
    checkProjectFile(path);
    if (!shouldIncludeFile(path)) {
        return;
    }
    const auto position =
        std::lower_bound(_fileCache.begin(), _fileCache.end(), path);
    if (position != _fileCache.end() && *position == path) {
        return;
    }
    _fileCache.insert(position, path);
    updateExtensions();
    emitFilesChanged();
}

void Project::removeCachedFile(std::filesystem::path path) {
    _tv();
    path = normalizedProjectPath(path);
    const auto end = std::remove_if(
        _fileCache.begin(), _fileCache.end(), [&](const auto &file) {
            const auto relative = file.lexically_relative(path);
            return !relative.empty() && !relative.is_absolute() &&
                   *relative.begin() != "..";
        });
    if (end != _fileCache.end()) {
        _fileCache.erase(end, _fileCache.end());
        updateExtensions();
        emitFilesChanged();
    }
}

void Project::updateCachedFile(std::filesystem::path path) {
    _tv();
    checkProjectFile(path);
}

void Project::checkProjectFile(std::filesystem::path path) {
    if (normalizedProjectPath(path) !=
        normalizedProjectPath(_settings.settingsPath)) {
        return;
    }
    const auto oldRoot = normalizedProjectPath(_settings.root);
    loadProjectFile();
    if (normalizedProjectPath(_settings.root) != oldRoot) {
        updateCache();
    }
}
