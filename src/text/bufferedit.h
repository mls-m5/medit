#pragma once

#include "cursor.h"
#include "fstring.h"
#include <vector>

class BufferEdit {
public:
    // TODO: Maybe create a better stringtype (basic_string does not work)
    FString from;
    FString to;
    Cursor position;

    void trim();

    bool empty() {
        return from.empty() && to.empty();
    }
};

BufferEdit revert(BufferEdit e);

/// Where `position` ends up after `edit` (trimmed) is applied, so that marks
/// and cursors follow the text around them:
/// - before the edit: unchanged;
/// - exactly at its start: after the inserted text when nothing is removed
///   (so lines inserted above a cursor keep it on its line), otherwise
///   unchanged;
/// - after the removed text: moved by the size difference;
/// - inside the removed text: kept where it was, but at most at the end of the
///   inserted text.
Position movePosition(Position position, const BufferEdit &edit);
