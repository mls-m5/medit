#pragma once

#include <filesystem>
#include <functional>

class DirectoryNotifications {
public:
    virtual ~DirectoryNotifications() = default;

    enum EventType { Added, Removed, Changed };

    using CallbackT = std::function<void(
        EventType, std::filesystem::path, std::filesystem::file_time_type)>;

    /// Set path to subscribe to
    virtual void path(std::filesystem::path) = 0;
    virtual void subscribe(CallbackT, void *ref) = 0;
    virtual void unsubcribe(void *ref) = 0;
};
