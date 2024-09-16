#pragma once

#include "core/os.h"
#include "files/directorynotifications.h"
#include "meditfwd.h"
#include <memory>

#ifdef MEDIT_USING_LINUX

std::unique_ptr<DirectoryNotifications> createDirectoryNotifications(
    IJobQueue &guiQueue);

#else

inline std::unique_ptr<DirectoryNotifications> createDirectoryNotifications(
    IJobQueue &guiQueue) {
    return {};
}

#endif
