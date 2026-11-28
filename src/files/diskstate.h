#pragma once

/// How a buffer's file on disk compares with what the buffer last loaded or
/// saved.
enum class DiskState {
    InSync,        // Unchanged, or nothing known about it
    ChangedOnDisk, // Changed by someone else
    DeletedOnDisk, // Removed by someone else
};
