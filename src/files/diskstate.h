#pragma once

#include <filesystem>
#include <optional>
#include <string>

/// How a buffer's file on disk compares with what the buffer last loaded or
/// saved.
enum class DiskState {
    InSync,        // Unchanged, or nothing known about it
    ChangedOnDisk, // Changed by someone else
    DeletedOnDisk, // Removed by someone else
};

/// A file's content as read at one moment.
struct DiskContent {
    bool exists = false;
    std::optional<std::string> bytes; // Nothing if missing or unreadable
};

/// Reads `path`. Safe to call on any thread.
DiskContent readDiskContent(const std::filesystem::path &path);
