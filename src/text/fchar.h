#pragma once

#include "utf8char.h"

using FormatType = int;

// A char with format
class FChar {
public:
    Utf8Char c;
    FormatType f = {};

    constexpr FChar() = default;
    constexpr FChar(const FChar &) = default;
    constexpr FChar(FChar &&) = default;
    constexpr FChar &operator=(const FChar &) = default;
    constexpr FChar &operator=(FChar &&) = default;

    constexpr FChar(char c, FormatType f = {}) : c(c), f(f) {}
    constexpr FChar(char *begin, size_t size) : c(begin, size) {}
    constexpr FChar(Utf8Char c) : c(c) {}

    constexpr auto size() const {
        return c.size();
    }

    operator std::string_view() const {
        return c;
    }

    operator std::string() const {
        return std::string{std::string_view{*this}};
    }
};
