#pragma once

#include "core/os.h"
#include "meditfwd.h"
#include <filesystem>
#include <functional>
#include <memory>

// TODO move to someowhere else
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

#ifdef MEDIT_USING_LINUX

std::unique_ptr<DirectoryNotifications> createDirectoryNotifications(
    IJobQueue &guiQueue);

#else

inline std::unique_ptr<DirectorynDirectoryNotifications>
createDirectoryNotifications(IJobQueue &guiQueue) {
    return {}
}

#endif
