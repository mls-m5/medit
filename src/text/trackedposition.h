#pragma once

#include "text/cursor.h"
#include "text/position.h"
#include <cstddef>
#include <memory>

class Buffer;

/// A position in a buffer that follows the text as it is edited: every change
/// to the buffer moves it (see movePosition), also undo, redo and reloading
/// from disk. Used for editor cursors and selections, and meant for more
/// cursors and other users' cursors later.
///
/// The buffer must be owned by a shared_ptr. The position stays valid if the
/// buffer goes away first; it then stays where it was last read.
class TrackedPosition {
public:
    TrackedPosition() = default;
    TrackedPosition(Buffer &buffer, Position position);
    TrackedPosition(const TrackedPosition &) = delete;
    TrackedPosition &operator=(const TrackedPosition &) = delete;
    TrackedPosition(TrackedPosition &&other) noexcept;
    TrackedPosition &operator=(TrackedPosition &&other) noexcept;
    ~TrackedPosition();

    [[nodiscard]] Position pos() const;
    void pos(Position position);

    /// The buffer it is tracked in, or null.
    [[nodiscard]] Buffer *buffer() const;

    /// As a cursor in its buffer. Requires a buffer.
    [[nodiscard]] Cursor cursor() const;

private:
    void release();

    std::weak_ptr<Buffer> _buffer;
    size_t _id = 0;
    mutable Position
        _fallback; // The last known position, used without a buffer
};
