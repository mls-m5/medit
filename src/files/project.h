#pragma once

#include "core/ijobqueue.h"
#include "core/threadvalidation.h"
#include "files/filesource.h"
#include "files/filewatcher.h"
#include "meditfwd.h"
#include "projectsettings.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

class IFileSystem;
class ICommandRunner;
class ITimer;

class Project {
public:
    enum ProjectLanguage {
        Cpp,
        Go,
        Unknown,
    };

    /// At most this many files are listed, so a huge directory cannot hang
    /// the editor.
    static constexpr size_t defaultFileLimit = 100000;

    /// Lists the files Git knows about (see createGitSource), or the
    /// directory outside a repository.
    ///
    /// The list is refreshed when the watched directories change (see
    /// watchFiles), shortly after the last change in a burst.
    Project(std::filesystem::path projectFile,
            IFileSystem &fileSystem,
            ICommandRunner &runner,
            IJobQueue &jobQueue,
            IJobQueue &guiQueue,
            ITimer &timer);
    ~Project();

    /// Lists the files now, on this thread (at startup).
    void refreshNow();

    /// Lists the files on the job thread and swaps them in on the GUI thread.
    /// Calls while one runs are coalesced into one more.
    void refresh();

    /// `callback` runs on the GUI thread whenever the file list changed. The
    /// subscription lasts as long as the returned handle.
    [[nodiscard]] std::shared_ptr<void> subscribeToFiles(
        std::function<void()> callback);

    /// Used by the next refresh.
    void fileLimit(size_t limit) {
        _fileLimit = limit;
    }

    /// At most this many directories are watched (inotify has a per-user
    /// limit); beyond that, changes are noticed at the next refresh.
    static constexpr size_t maxWatchedDirectories = 2000;

    /// Stops watching for changes. Called before the job queues stop, since
    /// the watcher posts to them.
    void stopWatching();

    /// The current snapshot.
    const ProjectFiles &projectFiles() const {
        return _files;
    }

    const ProjectSettings &settings() const {
        return _settings;
    }

    const std::vector<std::filesystem::path> &files() const {
        return _files.files;
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

    ProjectLanguage guessProjectLanguage() const;

    // Set the current root
    // std::filesystem::path findRoot(std::filesystem::path) const;

    /// When there is no build command provided. Try to guess what it could be
    /// based on what files are in the project
    std::string guessBuildCommand();

    /// Takes a new list in (GUI thread).
    void apply(ProjectFiles files);

    void updateExtensions();

    /// Watches the root, the directories with project files and the Git
    /// index and HEAD, for the current snapshot.
    void watchFiles();
    void fileChanged(const std::filesystem::path &path);

    ProjectSettings _settings;

    IFileSystem *_fileSystem = nullptr;
    std::shared_ptr<FileSource> _source; // Shared with a running listing
    ProjectFiles _files;
    size_t _fileLimit = defaultFileLimit;

    bool _refreshing = false;
    bool _refreshPending = false;

    std::vector<std::weak_ptr<std::function<void()>>> _subscribers;

    std::vector<std::pair<std::filesystem::path, size_t>> _extensions;

    ThreadValidation _tv;

    IJobQueue *_jobQueue = nullptr;
    IJobQueue *_guiQueue = nullptr;
    ITimer *_timer = nullptr;
    size_t _pendingRefresh = 0; // Timer id, 0 if none

    std::set<std::filesystem::path> _watched;

    /// Expires with this object; queued tasks check it before using `this`.
    std::shared_ptr<int> _alive = std::make_shared<int>();

    /// Last, so that it stops first when the project is destroyed.
    std::unique_ptr<FileWatcher> _watcher;
};
