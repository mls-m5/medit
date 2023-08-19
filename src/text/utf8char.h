#pragma once

#include <array>
#include <cstring> // for memcpy
#include <ostream>
#include <string_view>

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
    ~Utf8Char() = default;

    constexpr Utf8Char(char c) {
        _data.front() = c;
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

    constexpr Utf8Char(std::string_view data) {
        _data = {};
        for (size_t i = 0; i < data.size() && i < 4; ++i) {
            _data[i] = data[i];
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
            return {std::string_view{data, 2}, 2};
        }
        else if ((c & 0b11110000) == 0b11100000) {
            return {std::string_view{data, 3}, 3};
        }
        else if ((c & 0b11111000) == 0b11110000) {
            return {std::string_view{data, 4}, 4};
        }
        return {};
    }

    constexpr char operator[](size_t index) const {
        return _data.at(index);
    }

    constexpr char &operator[](size_t index) {
        return _data.at(index);
    }

    constexpr char at(size_t index) const {
        return _data.at(index);
    }

    constexpr char &at(size_t index) {
        return _data.at(index);
    }

    constexpr auto begin() {
        return _data.begin();
    }

    constexpr auto end() {
        return _data.end();
    }

    constexpr auto begin() const {
        return _data.begin();
    }

    constexpr auto end() const {
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

    operator uint32_t() const {
        auto value = uint32_t{};
        std::memcpy(&value, _data.data(), 4);
        return value;
    }

    friend bool operator<(const Utf8Char &self, const Utf8Char &other) {
        return static_cast<uint32_t>(self) < static_cast<uint32_t>(other);
    }

    friend constexpr bool operator==(const Utf8Char &self,
                                     const Utf8Char &other) {
        for (size_t i = 0; i < self._data.size(); ++i) {
            if (self._data[i] != other._data[i]) {
                return false;
            }
        }
        return true;
    }

    friend constexpr bool operator!=(const Utf8Char &self,
                                     const Utf8Char &other) {
        return !(self == other);
    }

    operator std::string_view() const {
        return std::string_view{_data.begin(), size()};
    }

    operator std::string() const {
        return std::string{_data.begin(), size()};
    }

    std::string toString() const {
        return std::string{_data.begin(), size()};
    }

    std::string byteRepresentation() const {
        std::string ret;
        for (auto c : _data) {
            ret += " " + std::to_string(static_cast<unsigned char>(c));
        }
        return ret;
    }

    friend std::ostream &operator<<(std::ostream &stream, Utf8Char &c) {
        return stream.write(&c._data.front(), static_cast<ptrdiff_t>(c.size()));
    }

    friend struct std::hash<Utf8Char>;
};

namespace std {
template <>
struct hash<Utf8Char> {
    std::size_t operator()(const Utf8Char &k) const {
        std::size_t h = 0;
        for (const auto &c : k._data) {
            h ^= std::hash<char>{}(c) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};
} // namespace std
