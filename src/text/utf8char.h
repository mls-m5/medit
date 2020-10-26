#pragma once

#include <array>

constexpr uint8_t utf8size(char first) {
    if ((first & 0b10000000) == 0) {
        return 1;
    }
    if ((first & 0b11100000) == 0b11000000) {
        return 2;
    }
    else if ((first & 0b11110000) == 0b11100000) {
        return 3;
    }
    else if ((first & 0b11111000) == 0b11110000) {
        return 4;
    }
    return 0;
}

class Utf8Char {
    std::array<char, 4> _data = {};

public:
    constexpr Utf8Char() = default;
    constexpr Utf8Char(const Utf8Char &) = default;
    constexpr Utf8Char(Utf8Char &&) = default;
    constexpr Utf8Char &operator=(const Utf8Char &) = default;
    constexpr Utf8Char &operator=(Utf8Char &&) = default;

    constexpr Utf8Char(char c) {
        _data.front() = c;
    }

    constexpr Utf8Char(const char *c, size_t size) {
        for (size_t i = 0; i < size && i < _data.size(); ++i) {
            _data[i] = c[i];
        }
    }

    constexpr Utf8Char(const char *data) {
        auto &c = *data;
        if ((c & 0b10000000) == 0) {
            _data = {c};
        }
        else if ((c & 0b11100000) == 0b11000000) {
            _data = {data[0], data[1]};
        }
        else if ((c & 0b11110000) == 0b11100000) {
            _data = {data[0], data[1], data[2]};
        }
        else if ((c & 0b11111000) == 0b11110000) {
            _data = {data[0], data[1], data[2], data[3]};
        }
    }

    //! Create a Utf8Char and also returns the size
    static constexpr inline std::pair<Utf8Char, size_t> fromChar(
        const char *data) {
        auto &c = *data;
        if ((c & 0b10000000) == 0) {
            return {{c}, 1};
        }
        else if ((c & 0b11100000) == 0b11000000) {
            return {{data, 2}, 2};
        }
        else if ((c & 0b11110000) == 0b11100000) {
            return {{data, 3}, 3};
        }
        else if ((c & 0b11111000) == 0b11110000) {
            return {{data, 4}, 4};
        }
        return {};
    }

    constexpr char operator[](size_t index) const {
        return _data[index];
    }

    constexpr char &operator[](size_t index) {
        return _data[index];
    }

    constexpr char at(size_t index) const {
        return _data[index];
    }

    constexpr char &at(size_t index) {
        return _data[index];
    }

    constexpr auto begin() {
        return _data.begin();
    }

    constexpr auto end() {
        return _data.end();
    }

    constexpr auto size() const {
        size_t sum = 0;
        for (auto byte : _data) {
            if (byte) {
                ++sum;
            }
        }

        return sum;
    }

    constexpr char &front() {
        return _data.front();
    }

    constexpr const char &front() const {
        return _data.front();
    }

    constexpr bool operator==(char c) const {
        return size() == 1 && _data.front() == c;
    }

    operator std::string_view() const {
        return std::string_view{_data.begin(), size()};
    }
};
