

#include "inotify.h"

#ifdef MEDIT_USING_LINUX

#include "core/ijobqueue.h"
#include "core/threadname.h"
#include "inotify.h"
#include <algorithm>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <limits.h>
#include <poll.h>
#include <sys/inotify.h>
#include <system_error>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {

class InotifyDirectoryNotifications : public DirectoryNotifications {
public:
    int inotify_fd = 0;
    int watch_desc = 0;
    std::atomic<bool> stop_thread{false};
    std::thread event_thread;
    IJobQueue &guiQueue;
    std::filesystem::path directory;

    InotifyDirectoryNotifications(const InotifyDirectoryNotifications &) =
        delete;
    InotifyDirectoryNotifications(InotifyDirectoryNotifications &&) = delete;
    InotifyDirectoryNotifications &operator=(
        const InotifyDirectoryNotifications &) = delete;
    InotifyDirectoryNotifications &operator=(InotifyDirectoryNotifications &&) =
        delete;

    InotifyDirectoryNotifications(IJobQueue &jobQueue)
        : inotify_fd(inotify_init())
        , guiQueue{jobQueue} {

        if (inotify_fd < 0) {
            std::cerr << "Error initializing inotify" << std::endl;
            return;
        }
    }

    ~InotifyDirectoryNotifications() override {
        stop();
        close(inotify_fd);
    }

    EventType decodeEventMask(uint32_t mask) {
        if (mask & IN_CREATE || mask & IN_MOVED_TO) {
            return EventType::Added;
        }
        else if (mask & IN_DELETE || mask & IN_MOVED_FROM) {
            return EventType::Removed;
        }
        else if (mask & IN_MODIFY) {
            return EventType::Changed;
        }
        else {
            throw std::runtime_error("Unsupported event mask");
        }
    }

    void handleEvent(struct inotify_event *event) {
        if (event) {
            EventType type = decodeEventMask(event->mask);
            std::filesystem::path filepath(directory / event->name);

            auto ec = std::error_code{};

            emitChanges(
                type, filepath, std::filesystem::last_write_time(filepath, ec));
        }
    }

    void emitChanges(EventType type,
                     const std::filesystem::path &filepath,
                     std::filesystem::file_time_type changedTime) {
        guiQueue.addTask([this, type, filepath, changedTime]() {
            for (const auto &subscriber : _subscribers) {
                subscriber.f(type, filepath, changedTime);
            }
        });
    }

    void path(std::filesystem::path directory) override {
        if (directory == this->directory) {
            return;
        }
        this->directory = directory;
        if (inotify_fd < 0) {
            return;
        }

        watch_desc = inotify_add_watch(inotify_fd,
                                       directory.c_str(),
                                       IN_CREATE | IN_DELETE | IN_MODIFY |
                                           IN_MOVED_FROM | IN_MOVED_TO);

        if (watch_desc < 0) {
            std::cerr << "Error adding inotify watch" << std::endl;
            return;
        }

        event_thread = std::thread([this]() {
            setThreadName("inotify");
            constexpr size_t buffer_size =
                sizeof(struct inotify_event) + NAME_MAX + 1;
            // char buffer[buffer_size];
            auto buffer = std::array<char, buffer_size>{};

            struct pollfd pfd = {};
            pfd.fd = inotify_fd;
            pfd.events = POLLIN;

            while (!stop_thread.load()) {
                int poll_result =
                    poll(&pfd, 1, 1000); // 1000ms (1 second) timeout

                if (poll_result < 0) {
                    perror("poll");
                    break;
                }

                if (poll_result == 0) {
                    // Timeout occurred, no events to process, check if the loop
                    // should continue
                    continue;
                }

                if (pfd.revents & POLLIN) {
                    int length = read(inotify_fd, buffer.data(), buffer.size());
                    if (length < 0) {
                        std::cerr << "Error reading inotify events"
                                  << std::endl;
                        break;
                    }

                    int i = 0;
                    while (i < length) {
                        struct inotify_event *event =
                            (struct inotify_event *)&buffer[i];
                        handleEvent(event);
                        i += sizeof(struct inotify_event) + event->len;
                    }
                }
            }
            inotify_rm_watch(inotify_fd, watch_desc);
        });
    }

    void subscribe(CallbackT f, void *ref) override {
        _subscribers.push_back({f, ref});
    }

    void unsubcribe(void *ref) override {
        auto it = std::remove_if(_subscribers.begin(),
                                 _subscribers.end(),
                                 [ref](auto &s) { return s.ref == ref; });
        _subscribers.erase(it, _subscribers.end());
    }

    void stop() {
        if (stop_thread.exchange(true)) {
            return; // Already stopped
        }
        if (event_thread.joinable()) {
            event_thread.join();
        }
    }

    std::filesystem::path _path;

    struct Subscriber {
        CallbackT f;
        void *ref = nullptr;
    };

    // Define thread here
    std::vector<Subscriber> _subscribers;
};

} // namespace

std::unique_ptr<DirectoryNotifications> createDirectoryNotifications(
    IJobQueue &jobQueue) {
    return std::make_unique<InotifyDirectoryNotifications>(jobQueue);
}

#endif
