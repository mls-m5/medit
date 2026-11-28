#pragma once

#include "files/diskstate.h"
#include "meditfwd.h"
#include <cstddef>
#include <string>

/// Compares the buffer's file on disk with its last load or save and marks the
/// buffer with the result (see Buffer::diskState). When the file changed, the
/// lines where the user's unsaved edits conflict with the change are marked.
DiskState checkDisk(Buffer &buffer);

/// Unchanged lines that must separate a change on disk from an unsaved
/// change for them to be merged automatically.
constexpr size_t mergeMargin = 3;

/// Merges a change on disk (`disk`) into the buffer's unsaved changes if they
/// are at least mergeMargin lines apart. @returns false (and changes nothing)
/// otherwise, or when the file is too large to have a merge base.
bool mergeFromDisk(Buffer &buffer, const std::string &disk);

enum class ReloadResult {
    Kept,  // Reloaded, merged, marked or already in sync
    Close, // The file was deleted and the buffer has no unsaved changes
};

/// Called when the buffer's file may have changed on disk. Reloads a buffer
/// without unsaved changes; merges into one with unsaved changes when the
/// changes are far enough apart (mergeFromDisk). Otherwise the buffer is never
/// replaced, only marked as with checkDisk. When the file was deleted, a buffer
/// with unsaved changes is marked and one without should be closed (the
/// caller does that).
ReloadResult reloadFromDisk(Buffer &buffer);

/// As above, with the file's content already read (e.g. on another thread).
ReloadResult reloadFromDisk(Buffer &buffer, const DiskContent &content);
