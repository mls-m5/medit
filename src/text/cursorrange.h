#pragma once

#include "buffer.h"
#include "cursor.h"

#include <utility>

//! A range between two cursor positions, that is guaranteed to be with begin
//! first and end second. It is also guaranteed to have a buffer that is not
//! null (assuming you do not put in a null reference)
class CursorRange {
    Buffer *_buffer;
    Position _begin;
    Position _end;

public:
    constexpr CursorRange(Buffer &buffer, Position begin, Position end)
        : _buffer(&buffer)
        , _begin(begin)
        , _end(end) {

        fixOrder();
    }

    //! The whole buffer
    CursorRange(Buffer &buffer)
        : _buffer(&buffer)
        , _begin(buffer.begin())
        , _end(buffer.end()) {}

    //! Convert a cursor to a empty range
    constexpr CursorRange(Cursor both)
        : CursorRange(both, both) {}

    constexpr CursorRange(Cursor begin, Cursor end)
        : CursorRange(begin.buffer(), begin, end) {}

    constexpr CursorRange(Cursor begin, Position end)
        : CursorRange(begin.buffer(), begin, end) {}

    constexpr CursorRange(const CursorRange &) = default;
    constexpr CursorRange(CursorRange &&) = default;
    constexpr CursorRange &operator=(const CursorRange &) = default;
    constexpr CursorRange &operator=(CursorRange &&) = default;

    constexpr void fixOrder() {
        if (_end < _begin) {
            std::swap(_begin, _end);
        }
    }

    constexpr Cursor begin() const {
        return {*_buffer, _begin.x(), _begin.y()};
    }

    constexpr auto beginPosition() const {
        return _begin;
    }

    constexpr Cursor end() const {
        return {*_buffer, _end.x(), _end.y()};
    }

    constexpr auto endPosition() const {
        return _end;
    }

    constexpr void beginPosition(Position value) {
        _begin = value;
        fixOrder();
    }

    constexpr void endPosition(Position value) {
        _end = value;
        fixOrder();
    }

    constexpr bool empty() const {
        return _begin == _end;
    }

    constexpr Buffer &buffer() {
        return *_buffer;
    }
};
