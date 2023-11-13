#pragma once

#include <cstddef>
#include <ostream>

class Position {
private:
    size_t _x;
    size_t _y;

public:
    static constexpr size_t max = static_cast<size_t>(-1);

    constexpr Position(size_t x = 0, size_t y = 0)
        : _x(x)
        , _y(y) {}
    constexpr Position(const Position &) = default;
    constexpr Position(Position &&) = default;
    constexpr Position &operator=(const Position &) = default;
    constexpr Position &operator=(Position &&) = default;
    ~Position() = default;
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

    friend constexpr bool operator<(const Position &self,
                                    const Position &other) {
        if (self.y() == other.y()) {
            return self.x() < other.x();
        }
        else {
            return self.y() < other.y();
        }
    }

    constexpr bool operator!=(const Position &other) const {
        return x() != other.x() || y() != other.y();
    }

    friend constexpr bool operator==(const Position &self,
                                     const Position &other) {
        return self.x() == other.x() && self.y() == other.y();
    }

    /// Move position one step forward. Only check if character is newspace or
    /// not, therefore not whole utf-character is needed
    constexpr Position &operator+=(char c) {
        if (c == '\n') {
            _x = 0;
            ++_y;
        }
        else {
            ++_x;
        }
        return *this;
    }
};

inline std::ostream &operator<<(std::ostream &stream,
                                const Position &position) {
    return stream << "(" << position.x() << ", " << position.y() << ")";
}
