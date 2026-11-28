#include "files/diskcheck.h"
#include "files/file.h"
#include "text/buffer.h"
#include "text/linemerge.h"

DiskState checkDisk(Buffer &buffer) {
    auto *file = buffer.file();
    if (!file) {
        return DiskState::InSync;
    }
    const auto state = file->diskState();
    auto conflicts = std::vector<LineRange>{};
    if (state == DiskState::ChangedOnDisk) {
        const auto *base = file->baseText();
        const auto disk = file->readDisk();
        if (base && disk) {
            conflicts = mergeLines(*base, fileBytes(buffer), *disk).conflicts;
        }
    }
    buffer.diskState(state, std::move(conflicts));
    return state;
}

void reloadFromDisk(Buffer &buffer) {
    auto *file = buffer.file();
    if (!file) {
        return;
    }
    if (buffer.isChanged() || !file->readDisk()) {
        checkDisk(buffer);
        return;
    }
    buffer.load();
}
