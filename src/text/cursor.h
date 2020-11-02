#pragma once

#include "meditfwd.h"
#include <cstddef>

struct Cursor {
private:
    Buffer *_buffer;
    size_t _x;
    size_t _y;

public:
    constexpr auto x() const {
        return _x;
    }


    constexpr auto y() const {
        return _y;
    }


    constexpr Cursor &x(size_t value) {
        _x = value;
        return *this;
    }

    constexpr Cursor &y(size_t value) {
        _y = value;
        return *this;
    }

    constexpr Buffer &buffer() {
        return *_buffer;
    }

    constexpr Cursor(Buffer &buffer, size_t x = 0, size_t y = 0)
        : _buffer(&buffer), _x(x), _y(y) {}

    Cursor(const Cursor &) = default;
    Cursor(Cursor &&) = default;
    Cursor &operator=(const Cursor &) = default;
    Cursor &operator=(Cursor &&) = default;
};
