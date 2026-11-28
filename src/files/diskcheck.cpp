#include "files/diskcheck.h"
#include "core/meditlog.h"
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
            // The same margin as for merging, so the lines that kept the
            // change from being merged are the ones marked.
            conflicts =
                mergeLines(
                    *base, fileBytes(buffer), *content.bytes, mergeMargin)
                    .conflicts;
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

bool mergeFromDisk(Buffer &buffer, const std::string &disk) {
    const auto *base = buffer.file()->baseText();
    if (!base) {
        return false; // Too large to keep a merge base.
    }
    const auto ours = fileBytes(buffer);
    const auto merge = mergeLines(*base, ours, disk, mergeMargin);
    if (merge.conflict) {
        return false;
    }
    auto merged = applyLineEdits(ours, merge.edits);
    if (ours.size() != buffer.text().size() && !merged.empty() &&
        merged.back() == '\n') {
        // fileBytes added the final newline; the buffer does not have it.
        merged.pop_back();
    }
    buffer.mergeFromDisk(merged, disk);
    return true;
}

ReloadResult reloadFromDisk(Buffer &buffer) {
    if (auto *file = buffer.file()) {
        return reloadFromDisk(buffer, readDiskContent(file->path()));
    }
    return ReloadResult::Kept;
}

ReloadResult reloadFromDisk(Buffer &buffer, const DiskContent &content) {
    auto *file = buffer.file();
    if (!file) {
        return ReloadResult::Kept;
    }
    const auto state = file->diskState(content);
    const auto previous = buffer.diskState();
    const auto name = file->path().filename().string();
    if (state == DiskState::ChangedOnDisk && content.bytes) {
        if (!buffer.isChanged()) {
            buffer.reload(*content.bytes);
            logStatusMessage(name, " reloaded from disk");
            return ReloadResult::Kept;
        }
        if (mergeFromDisk(buffer, *content.bytes)) {
            logStatusMessage(
                name,
                " changed on disk; merged with your unsaved changes (undo to "
                "revert)");
            return ReloadResult::Kept;
        }
    }
    if (state == DiskState::DeletedOnDisk && !buffer.isChanged()) {
        logStatusMessage(name, " was deleted on disk; closed");
        return ReloadResult::Close;
    }
    // Never replace unsaved changes: only mark the buffer.
    markDiskState(buffer, state, content);
    if (previous != state) {
        if (state == DiskState::ChangedOnDisk) {
            logStatusMessage(name,
                             " changed on disk and conflicts with your "
                             "unsaved changes");
        }
        else if (state == DiskState::DeletedOnDisk) {
            logStatusMessage(name,
                             " was deleted on disk; save writes it again");
        }
    }
    return ReloadResult::Kept;
}
