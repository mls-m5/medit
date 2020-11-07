#pragma once

#include "meditfwd.h"
#include <cstddef>

class Position {
private:
    size_t _x;
    size_t _y;

public:
    constexpr Position(size_t x = 0, size_t y = 0) : _x(x), _y(y) {}
    constexpr Position(const Position &) = default;
    constexpr Position(Position &&) = default;
    constexpr Position &operator=(const Position &) = default;
    constexpr Position &operator=(Position &&) = default;
    constexpr auto x() const {
        return _x;
    }

    constexpr auto y() const {
        return _y;
    }

    constexpr Position &x(size_t value) {
        _x = value;
        return *this;
    }

    constexpr Position &y(size_t value) {
        _y = value;
        return *this;
    }

    constexpr bool operator<(const Position &other) const {
        if (y() == other.y()) {
            return x() < other.x();
        }
        else {
            return y() < other.y();
        }
    }
};

class Cursor : public Position {
private:
    Buffer *_buffer;

public:
    constexpr Buffer &buffer() {
        return *_buffer;
    }

    constexpr Cursor(Buffer &buffer, size_t x = 0, size_t y = 0)
        : Position(x, y), _buffer(&buffer) {}

    constexpr Cursor(const Cursor &) = default;
    constexpr Cursor(Cursor &&) = default;
    constexpr Cursor &operator=(const Cursor &) = default;
    constexpr Cursor &operator=(Cursor &&) = default;
};
