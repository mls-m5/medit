#pragma once

#include <filesystem>
#include <functional>
#include <memory>

/// Watches the directories of open files on its own thread and reports paths
/// in them that may have changed. Directories rather than files, since many
/// programs save by writing a new file and renaming it over the old one.
///
/// Reports are hints: they may be duplicated, and the receiver re-reads the
/// file to decide.
class FileWatcher {
public:
    /// Called on the watcher's thread.
    using CallbackT = std::function<void(const std::filesystem::path &)>;

    FileWatcher() = default;
    FileWatcher(const FileWatcher &) = delete;
    FileWatcher &operator=(const FileWatcher &) = delete;
    virtual ~FileWatcher() = default;

    /// Start watching `directory`; counted, so each call needs an unwatch.
    virtual void watch(const std::filesystem::path &directory) = 0;
    virtual void unwatch(const std::filesystem::path &directory) = 0;

    /// Stops the thread. No callbacks are made after this returns.
    virtual void stop() = 0;
};

/// The Linux (inotify) watcher, or one that watches nothing elsewhere.
std::unique_ptr<FileWatcher> createFileWatcher(FileWatcher::CallbackT callback);
