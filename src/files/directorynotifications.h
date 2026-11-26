#pragma once

#include <filesystem>
#include <functional>

class DirectoryNotifications {
public:
    DirectoryNotifications() = default;
    DirectoryNotifications(const DirectoryNotifications &) = delete;
    DirectoryNotifications(DirectoryNotifications &&) = delete;
    DirectoryNotifications &operator=(const DirectoryNotifications &) = delete;
    DirectoryNotifications &operator=(DirectoryNotifications &&) = delete;
    virtual ~DirectoryNotifications() = default;

    // Rescan requests a fresh snapshot after directory changes or lost events.
    enum EventType { Added, Removed, Changed, Rescan };

    using CallbackT = std::function<void(
        EventType, std::filesystem::path, std::filesystem::file_time_type)>;

    /// GUI-thread API; callbacks run on the supplied GUI queue, which must
    /// outlive this object. Observe a root recursively; empty path stops
    /// watching. Root changes and destruction invalidate queued events from the
    /// old watch.
    virtual void path(std::filesystem::path) = 0;
    virtual void subscribe(CallbackT, void *ref) = 0;
    virtual void unsubcribe(void *ref) = 0;
};
