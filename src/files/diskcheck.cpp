#include "files/diskcheck.h"
#include "files/file.h"
#include "text/buffer.h"
#include "text/linemerge.h"

namespace {

void markDiskState(Buffer &buffer,
                   DiskState state,
                   const DiskContent &content) {
    auto conflicts = std::vector<LineRange>{};
    if (state == DiskState::ChangedOnDisk) {
        const auto *base = buffer.file()->baseText();
        if (base && content.bytes) {
            conflicts =
                mergeLines(*base, fileBytes(buffer), *content.bytes).conflicts;
        }
    }
    buffer.diskState(state, std::move(conflicts));
}

} // namespace

DiskState checkDisk(Buffer &buffer) {
    auto *file = buffer.file();
    if (!file) {
        return DiskState::InSync;
    }
    const auto content = readDiskContent(file->path());
    const auto state = file->diskState(content);
    markDiskState(buffer, state, content);
    return state;
}

void reloadFromDisk(Buffer &buffer) {
    if (auto *file = buffer.file()) {
        reloadFromDisk(buffer, readDiskContent(file->path()));
    }
}

void reloadFromDisk(Buffer &buffer, const DiskContent &content) {
    auto *file = buffer.file();
    if (!file) {
        return;
    }
    const auto state = file->diskState(content);
    if (state == DiskState::ChangedOnDisk && !buffer.isChanged() &&
        content.bytes) {
        buffer.reload(*content.bytes);
        return;
    }
    // Never replace unsaved changes: only mark the buffer.
    markDiskState(buffer, state, content);
}
