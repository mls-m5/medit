#pragma once

#include "files/diskstate.h"
#include "meditfwd.h"

/// Compares the buffer's file on disk with its last load or save and marks the
/// buffer with the result (see Buffer::diskState). When the file changed, the
/// lines where the user's unsaved edits conflict with the change are marked.
DiskState checkDisk(Buffer &buffer);

/// Called when the buffer's file may have changed on disk. Reloads a buffer
/// without unsaved changes; a buffer with unsaved changes is never replaced,
/// only marked as with checkDisk.
void reloadFromDisk(Buffer &buffer);
