#pragma once

#include "core/ijobqueue.h"
#include "core/threadvalidation.h"
#include "meditfwd.h"
#include "projectsettings.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class Project {
public:
    enum ProjectLanguage {
        Cpp,
        Go,
        Unknown,
    };

    Project(std::filesystem::path projectFile,
            DirectoryNotifications &files,
            IJobQueue &guiQueue);
    ~Project();

    Project(const Project &) = delete;
    Project &operator=(const Project &) = delete;
    Project(Project &&) = delete;
    Project &operator=(Project &&) = delete;

    // GUI-thread API. Callbacks see the updated list; subscribing does not
    // invoke the callback. Unsubscribe before destroying the subscriber.
    using FilesChangedCallbackT = std::function<void()>;
    void subscribeToFilesChanged(FilesChangedCallbackT callback, void *ref);
    void unsubscribeFromFilesChanged(void *ref);

    void updateCache(size_t max = 100000);

    const ProjectSettings &settings() const {
        return _settings;
    }

    const auto &files() const {
        _tv();
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

    size_t indentWidth() const {
        return 4;
    }

private:
    void loadProjectFile();
    bool shouldIncludeFile(const std::filesystem::path &path) const;
    void updateExtensions();
    void emitFilesChanged();

    ProjectLanguage guessProjectLanguage() const;

    // Set the current root
    // std::filesystem::path findRoot(std::filesystem::path) const;

    /// When there is no build command provided. Try to guess what it could be
    /// based on what files are in the project
    std::string guessBuildCommand();

    std::vector<std::filesystem::path> _fileCache;

    std::vector<std::filesystem::path> findProjectFiles(
        const std::filesystem::path &pathInProject, size_t max = 100000);

    ProjectSettings _settings;

    void addCachedFile(std::filesystem::path);
    void removeCachedFile(std::filesystem::path);
    void updateCachedFile(std::filesystem::path);

    /// Check if the projcet file needs to be reloaded
    void checkProjectFile(std::filesystem::path);

    std::vector<std::pair<std::filesystem::path, size_t>> _extensions;

    ThreadValidation _tv;

    // Both the notifier and GUI queue must outlive Project. Queued events
    // check this token on the GUI thread before accessing the project.
    DirectoryNotifications *_directoryNotifications = nullptr;
    std::filesystem::path _watchedRoot;
    std::shared_ptr<int> _lifetime = std::make_shared<int>(0);

    struct FilesSubscriber {
        FilesChangedCallbackT callback;
        void *ref;
        bool active = true;
    };
    std::vector<std::shared_ptr<FilesSubscriber>> _filesSubscribers;
};
