#include "files/filewatcher.h"
#include "core/os.h"

#ifdef MEDIT_USING_LINUX

#include "core/threadname.h"
#include <array>
#include <atomic>
#include <cerrno>
#include <climits>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <mutex>
#include <poll.h>
#include <sys/inotify.h>
#include <thread>
#include <unistd.h>

namespace {

class InotifyFileWatcher : public FileWatcher {
public:
    explicit InotifyFileWatcher(CallbackT callback)
        : _callback{std::move(callback)}
        , _fd{inotify_init1(IN_NONBLOCK | IN_CLOEXEC)} {
        if (_fd < 0) {
            std::cerr << "medit: cannot watch files: " << std::strerror(errno)
                      << std::endl;
            return;
        }
        if (pipe2(_stopPipe.data(), O_CLOEXEC) != 0) {
            close(_fd);
            _fd = -1;
            return;
        }
        _thread = std::thread{[this] { loop(); }};
    }

    ~InotifyFileWatcher() override {
        stop();
        if (_fd >= 0) {
            close(_fd);
            close(_stopPipe[0]);
            close(_stopPipe[1]);
        }
    }

    void watch(const std::filesystem::path &directory) override {
        if (_fd < 0) {
            return;
        }
        auto lock = std::lock_guard{_mutex};
        if (_counts[directory]++ > 0) {
            return;
        }
        // IN_CLOSE_WRITE rather than IN_MODIFY: one event when a writer is
        // done instead of one per write.
        const auto wd =
            inotify_add_watch(_fd,
                              directory.c_str(),
                              IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE |
                                  IN_DELETE | IN_MOVED_FROM | IN_ONLYDIR);
        if (wd < 0) {
            _counts.erase(directory);
            return;
        }
        _directories[wd] = directory;
    }

    void unwatch(const std::filesystem::path &directory) override {
        auto lock = std::lock_guard{_mutex};
        auto count = _counts.find(directory);
        if (count == _counts.end() || --count->second > 0) {
            return;
        }
        _counts.erase(count);
        for (auto it = _directories.begin(); it != _directories.end(); ++it) {
            if (it->second == directory) {
                inotify_rm_watch(_fd, it->first);
                _directories.erase(it);
                break;
            }
        }
    }

    void stop() override {
        if (_stopping.exchange(true)) {
            return;
        }
        if (_thread.joinable()) {
            const char c = 0;
            [[maybe_unused]] auto ignored = write(_stopPipe[1], &c, 1);
            _thread.join();
        }
    }

private:
    void loop() {
        setThreadName("file watcher");
        // Room for many events with names of any length.
        alignas(inotify_event) auto buffer =
            std::array<char, 64 * (sizeof(inotify_event) + NAME_MAX + 1)>{};
        auto fds = std::array<pollfd, 2>{
            {{_fd, POLLIN, 0}, {_stopPipe[0], POLLIN, 0}}};
        while (!_stopping) {
            if (poll(fds.data(), fds.size(), -1) < 0) {
                if (errno == EINTR) {
                    continue;
                }
                break;
            }
            if (fds[1].revents || _stopping) {
                break;
            }
            for (;;) {
                const auto length = read(_fd, buffer.data(), buffer.size());
                if (length <= 0) {
                    break; // EAGAIN: read everything.
                }
                for (ssize_t i = 0; i < length;) {
                    const auto *event =
                        reinterpret_cast<const inotify_event *>(&buffer[i]);
                    handle(*event);
                    i += static_cast<ssize_t>(sizeof(inotify_event) +
                                              event->len);
                }
            }
        }
    }

    void handle(const inotify_event &event) {
        // Directories too: a new directory can hold new project files.
        if (!event.len) {
            return;
        }
        auto path = std::filesystem::path{};
        {
            auto lock = std::lock_guard{_mutex};
            auto it = _directories.find(event.wd);
            if (it == _directories.end()) {
                return;
            }
            path = it->second / event.name;
        }
        if (!_stopping) {
            _callback(path);
        }
    }

    CallbackT _callback;
    int _fd = -1;
    std::array<int, 2> _stopPipe{-1, -1};
    std::atomic_bool _stopping = false;
    std::mutex _mutex;
    std::map<std::filesystem::path, int> _counts;
    std::map<int, std::filesystem::path> _directories;
    std::thread _thread;
};

} // namespace

std::unique_ptr<FileWatcher> createFileWatcher(
    FileWatcher::CallbackT callback) {
    return std::make_unique<InotifyFileWatcher>(std::move(callback));
}

#else

namespace {

class NullFileWatcher : public FileWatcher {
public:
    void watch(const std::filesystem::path &) override {}
    void unwatch(const std::filesystem::path &) override {}
    void stop() override {}
};

} // namespace

std::unique_ptr<FileWatcher> createFileWatcher(FileWatcher::CallbackT) {
    return std::make_unique<NullFileWatcher>();
}

#endif
