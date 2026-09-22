#pragma once

#include "meditfwd.h"
#include "text/position.h"

class Cursor : public Position {
private:
    Buffer *_buffer;

public:
    constexpr Buffer &buffer() {
        return *_buffer;
    }

    constexpr Buffer &buffer() const {
        return *_buffer;
    }

    constexpr Cursor(Buffer &buffer, size_t x = 0, size_t y = 0)
        : Position(x, y)
        , _buffer(&buffer) {}

    constexpr Cursor(Buffer &buffer, Position pos)
        : Position(pos)
        , _buffer(&buffer) {}

    constexpr Cursor(const Cursor &) = default;
    constexpr Cursor(Cursor &&) = default;
    constexpr Cursor &operator=(const Cursor &) = default;
    constexpr Cursor &operator=(Cursor &&) = default;

    ~Cursor() = default;

    Cursor &operator++();

    //! Returns a pointer because somtimes there is no character (like when
    //! there is a line-break
    FChar operator*();

    constexpr Position pos() const {
        return Position{x(), y()};
    }
};
