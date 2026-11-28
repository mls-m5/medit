#include "text/trackedposition.h"
#include "text/buffer.h"
#include <stdexcept>
#include <utility>

TrackedPosition::TrackedPosition(Buffer &buffer, Position position)
    : _buffer{buffer.weak_from_this()}
    , _fallback{position} {
    if (auto locked = _buffer.lock()) {
        _id = locked->addAnchor(position);
    }
}

TrackedPosition::TrackedPosition(TrackedPosition &&other) noexcept
    : _buffer{std::move(other._buffer)}
    , _id{std::exchange(other._id, 0)}
    , _fallback{other._fallback} {}

TrackedPosition &TrackedPosition::operator=(TrackedPosition &&other) noexcept {
    if (this != &other) {
        release();
        _buffer = std::move(other._buffer);
        _id = std::exchange(other._id, 0);
        _fallback = other._fallback;
    }
    return *this;
}

TrackedPosition::~TrackedPosition() {
    release();
}

void TrackedPosition::release() {
    if (_id) {
        if (auto locked = _buffer.lock()) {
            _fallback = locked->anchor(_id);
            locked->removeAnchor(_id);
        }
        _id = 0;
    }
}

Position TrackedPosition::pos() const {
    if (_id) {
        if (auto locked = _buffer.lock()) {
            _fallback = locked->anchor(_id);
        }
    }
    return _fallback;
}

void TrackedPosition::pos(Position position) {
    _fallback = position;
    if (_id) {
        if (auto locked = _buffer.lock()) {
            locked->anchor(_id, position);
        }
    }
}

Buffer *TrackedPosition::buffer() const {
    return _buffer.lock().get();
}

Cursor TrackedPosition::cursor() const {
    auto *buffer = this->buffer();
    if (!buffer) {
        throw std::runtime_error{"tracked position without a buffer"};
    }
    return {*buffer, pos()};
}
