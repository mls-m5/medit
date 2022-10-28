#pragma once

#include "meditfwd.h"
#include "text/position.h"

class Cursor : public Position {
private:
    RawBuffer *_buffer;

public:
    constexpr RawBuffer &buffer() {
        return *_buffer;
    }

    constexpr RawBuffer &buffer() const {
        return *_buffer;
    }

    constexpr Cursor(RawBuffer &buffer, size_t x = 0, size_t y = 0)
        : Position(x, y), _buffer(&buffer) {}

    constexpr Cursor(RawBuffer &buffer, Position pos)
        : Position(pos), _buffer(&buffer) {}

    constexpr Cursor(const Cursor &) = default;
    constexpr Cursor(Cursor &&) = default;
    constexpr Cursor &operator=(const Cursor &) = default;
    constexpr Cursor &operator=(Cursor &&) = default;

    Cursor &operator++();

    //! Returns a pointer because somtimes there is no character (like when
    //! there is a line-break
    FChar *operator*();

    FChar *operator->() {
        return **this;
    }
};
