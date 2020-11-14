#pragma once

#include <string_view>

class Color {
public:
    constexpr Color() = default;
    constexpr Color(const Color &) = default;
    constexpr Color(Color &&) = default;
    constexpr Color &operator=(const Color &) = default;
    constexpr Color &operator=(Color &&) = default;

    constexpr Color(std::string_view color) {
        if (color.size() == 7) {
            _r = stoi({&color.at(1), 2});
            _g = stoi({&color.at(3), 2});
            _b = stoi({&color.at(5), 2});
        }
    }

    constexpr auto r() const {
        return _r;
    }

    constexpr auto g() const {
        return _g;
    }

    constexpr auto b() const {
        return _b;
    }

private:
    unsigned char _r = 0, _g = 0, _b = 0;

    static unsigned char stoi(std::string_view str) {
        return ctoi(str.front()) * 16 + ctoi(str.back());
    }

    static unsigned char ctoi(char c) {
        if (c >= '0' && c <= '9') {
            return c - '0';
        }
        else if (c >= 'A' && c <= 'Z') {
            return c - 'A' + 10;
        }
        else if (c >= 'a' && c <= 'z') {
            return c - 'a' + 10;
        }
        return 0;
    }
};
