#include "inotify.h"

#ifdef MEDIT_USING_LINUX

#include "core/ijobqueue.h"
#include "core/threadname.h"
#include "core/threadvalidation.h"
#include "files/path.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <iostream>
#include <map>
#include <poll.h>
#include <set>
#include <sys/inotify.h>
#include <system_error>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {

bool inotifyContainsPath(const std::filesystem::path &parent,
                         const std::filesystem::path &path) {
    const auto relative = path.lexically_relative(parent);
    return !relative.empty() && !relative.is_absolute() &&
           *relative.begin() != "..";
}

class InotifyDirectoryNotifications : public DirectoryNotifications {
public:
    explicit InotifyDirectoryNotifications(IJobQueue &guiQueue)
        : _guiQueue{guiQueue} {}

    ~InotifyDirectoryNotifications() override {
        stop();
        _generation.reset();
        _lifetime.reset();
        for (const auto &subscriber : _subscribers)
            subscriber->active = false;
    }

    void path(std::filesystem::path directory) override {
        _tv();
        directory = normalizedAbsolutePath(directory);
        if (directory == _directory)
            return;
        stop();
        _generation = std::make_shared<Generation>();
        _directory = std::move(directory);
        _retry = false;
        _reportedFailure = false;
        if (_directory.empty())
            return;
        _stopping = false;
        _fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
        if (_fd < 0)
            failure(_directory,
                    std::error_code{errno, std::generic_category()});
        else
            addDirectoryTree(_directory);
        if (_retry)
            emitChanges(Rescan, _directory);
        _thread = std::thread{[this] { run(); }};
    }

    void subscribe(CallbackT callback, void *ref) override {
        _tv();
        _subscribers.push_back(
            std::make_shared<Subscriber>(Subscriber{std::move(callback), ref}));
    }

    void unsubcribe(void *ref) override {
        _tv();
        std::erase_if(_subscribers, [ref](const auto &subscriber) {
            if (subscriber->ref != ref)
                return false;
            subscriber->active = false;
            return true;
        });
    }

private:
    struct Subscriber {
        CallbackT callback;
        void *ref;
        bool active = true;
    };
    struct Generation {
        std::atomic<bool> refreshQueued{false};
    };

    IJobQueue &_guiQueue;
    ThreadValidation _tv{"directory notifications GUI thread"};
    std::vector<std::shared_ptr<Subscriber>> _subscribers;
    std::shared_ptr<int> _lifetime = std::make_shared<int>(0);
    std::shared_ptr<Generation> _generation = std::make_shared<Generation>();
    std::filesystem::path _directory;
    std::map<int, std::filesystem::path> _watches;
    std::set<std::filesystem::path> _watchedDirectories;
    int _fd = -1;
    std::atomic<bool> _stopping{true};
    std::thread _thread;
    // Watch state belongs to the worker, except while stopped or before start.
    bool _retry = false;
    bool _reportedFailure = false;

    void stop() {
        _stopping = true;
        if (_thread.joinable())
            _thread.join();
        if (_fd >= 0)
            close(_fd);
        _fd = -1;
        _watches.clear();
        _watchedDirectories.clear();
    }

    void failure(const std::filesystem::path &path, std::error_code error) {
        // A child may disappear or be replaced after its create event/stat.
        // Its parent watch covers future entries; this is not degraded
        // watching. Losing the selected root itself still requires recovery.
        if (path != _directory &&
            (error == std::errc::no_such_file_or_directory ||
             error == std::errc::not_a_directory))
            return;
        _retry = true;
        if (!_reportedFailure) {
            std::cerr << "Cannot watch " << path << ": " << error.message()
                      << "; retrying watches and refreshing the project every "
                         "second\n";
            _reportedFailure = true;
        }
    }

    bool addWatch(const std::filesystem::path &path) {
        if (_watchedDirectories.contains(path))
            return true;
        constexpr auto mask = IN_CREATE | IN_DELETE | IN_MODIFY | IN_ATTRIB |
                              IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE_SELF |
                              IN_MOVE_SELF;
        // An explicitly selected root may be a symlink, but never follow child
        // directory symlinks (including a replacement racing with traversal).
        const auto flags =
            IN_ONLYDIR | (path == _directory ? 0 : IN_DONT_FOLLOW);
        const auto descriptor =
            inotify_add_watch(_fd, path.c_str(), mask | flags);
        if (descriptor < 0) {
            failure(path, std::error_code{errno, std::generic_category()});
            return false;
        }
        if (const auto old = _watches.find(descriptor); old != _watches.end())
            _watchedDirectories.erase(old->second);
        _watches[descriptor] = path;
        _watchedDirectories.insert(path);
        return true;
    }

    void addDirectoryTree(const std::filesystem::path &root) {
        // Walk one directory at a time. A recursive iterator can become end()
        // on a vanished child and leave all remaining siblings unwatched.
        auto pending = std::vector<std::filesystem::path>{root};
        while (!_stopping && !pending.empty()) {
            const auto directory = std::move(pending.back());
            pending.pop_back();
            std::error_code error;
            const auto status =
                directory == _directory
                    ? std::filesystem::status(directory, error)
                    : std::filesystem::symlink_status(directory, error);
            if (error) {
                failure(directory, error);
                continue;
            }
            if (!std::filesystem::is_directory(status)) {
                if (directory == _directory)
                    failure(directory,
                            std::make_error_code(std::errc::not_a_directory));
                continue;
            }
            if (!addWatch(directory))
                continue;
            auto it = std::filesystem::directory_iterator{directory, error};
            if (error) {
                failure(directory, error);
                continue;
            }
            while (!_stopping && it != std::filesystem::directory_iterator{}) {
                const auto child = it->path();
                const auto childStatus = it->symlink_status(error);
                if (error)
                    failure(child, error);
                else if (std::filesystem::is_directory(childStatus))
                    pending.push_back(child);
                error.clear();
                it.increment(error);
                if (error) {
                    failure(directory, error);
                    break;
                }
            }
        }
    }

    void removeDirectoryTree(const std::filesystem::path &root) {
        std::erase_if(_watches, [&](const auto &watch) {
            if (!inotifyContainsPath(root, watch.second))
                return false;
            inotify_rm_watch(_fd, watch.first);
            _watchedDirectories.erase(watch.second);
            return true;
        });
    }

    void emitChanges(EventType type, const std::filesystem::path &path) {
        if (type == Rescan && _generation->refreshQueued.exchange(true))
            return;
        std::error_code error;
        const auto time = std::filesystem::last_write_time(path, error);
        _guiQueue.addTask([this,
                           lifetime = std::weak_ptr{_lifetime},
                           generation = std::weak_ptr{_generation},
                           type,
                           path,
                           time] {
            const auto currentGeneration = generation.lock();
            if (lifetime.expired() || !currentGeneration)
                return;
            _tv();
            if (type == Rescan)
                currentGeneration->refreshQueued = false;
            const auto subscribers = _subscribers;
            for (const auto &subscriber : subscribers)
                if (subscriber->active && subscriber->callback)
                    subscriber->callback(type, path, time);
        });
    }

    void streamFailure(std::error_code error) {
        if (_fd >= 0)
            close(_fd);
        _fd = -1;
        _watches.clear();
        _watchedDirectories.clear();
        failure(_directory, error);
        emitChanges(Rescan, _directory);
    }

    void recover() {
        // Also catches directories populated before their watches were
        // installed.
        _retry = false;
        if (_fd < 0)
            _fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
        if (_fd < 0)
            failure(_directory,
                    std::error_code{errno, std::generic_category()});
        else
            addDirectoryTree(_directory);
        if (!_retry)
            _reportedFailure = false;
        emitChanges(Rescan, _directory);
    }

    void handleEvent(const inotify_event &event) {
        if (event.mask & IN_Q_OVERFLOW) {
            removeDirectoryTree(_directory);
            recover();
            return;
        }
        const auto found = _watches.find(event.wd);
        if (found == _watches.end())
            return; // includes explicitly removed watches
        const auto directory = found->second;
        if (event.mask &
            (IN_IGNORED | IN_DELETE_SELF | IN_MOVE_SELF | IN_UNMOUNT)) {
            removeDirectoryTree(directory);
            // Normal child deletion/moves are covered by the parent watch.
            // Root loss or an unexpected lost watch still needs recovery.
            if (directory == _directory ||
                !(event.mask & (IN_DELETE_SELF | IN_MOVE_SELF)))
                _retry = true;
            emitChanges(Rescan, _directory);
            return;
        }
        if (event.len == 0)
            return;
        const auto path = directory / event.name;
        if (event.mask & IN_ISDIR) {
            if (event.mask & (IN_DELETE | IN_MOVED_FROM))
                removeDirectoryTree(path);
            if (event.mask & (IN_CREATE | IN_MOVED_TO | IN_ATTRIB))
                addDirectoryTree(path);
            emitChanges(Rescan, _directory);
        }
        else if (event.mask & (IN_CREATE | IN_MOVED_TO))
            emitChanges(Added, path);
        else if (event.mask & (IN_DELETE | IN_MOVED_FROM))
            emitChanges(Removed, path);
        else if (event.mask & (IN_MODIFY | IN_ATTRIB))
            emitChanges(Changed, path);
    }

    void run() {
        setThreadName("inotify");
        auto retryAt =
            std::chrono::steady_clock::now() + std::chrono::seconds{1};
        // The buffer must be suitably aligned for inotify_event, not just char.
        alignas(inotify_event) std::array<char, 64 * 1024> buffer{};
        while (!_stopping) {
            pollfd descriptor{_fd, POLLIN, 0};
            const auto ready = poll(&descriptor, 1, 50);
            if (_stopping)
                break;
            if (ready < 0 && errno != EINTR) {
                streamFailure(std::error_code{errno, std::generic_category()});
            }
            else if (ready > 0 &&
                     (descriptor.revents & (POLLERR | POLLHUP | POLLNVAL))) {
                streamFailure(std::make_error_code(std::errc::io_error));
            }
            else if (ready > 0 && (descriptor.revents & POLLIN)) {
                const auto count = read(_fd, buffer.data(), buffer.size());
                if (count < 0 && errno != EAGAIN && errno != EINTR)
                    streamFailure(
                        std::error_code{errno, std::generic_category()});
                else if (count == 0)
                    streamFailure(std::make_error_code(std::errc::io_error));
                for (size_t offset = 0; !_stopping && count > 0 &&
                                        offset + sizeof(inotify_event) <=
                                            static_cast<size_t>(count);) {
                    const auto &event =
                        *reinterpret_cast<const inotify_event *>(buffer.data() +
                                                                 offset);
                    if (offset + sizeof(inotify_event) + event.len >
                        static_cast<size_t>(count))
                        break;
                    handleEvent(event);
                    offset += sizeof(inotify_event) + event.len;
                }
            }
            if (_retry && std::chrono::steady_clock::now() >= retryAt) {
                recover();
                retryAt =
                    std::chrono::steady_clock::now() + std::chrono::seconds{1};
            }
        }
    }
};

} // namespace

std::unique_ptr<DirectoryNotifications> createDirectoryNotifications(
    IJobQueue &guiQueue) {
    return std::make_unique<InotifyDirectoryNotifications>(guiQueue);
}

#endif
