#pragma once

#include "utf8char.h"

#include "formattype.h"

// A char with format
class FChar {
public:
    Utf8Char c;
    FormatType f = 1;

    constexpr FChar() = default;
    constexpr FChar(const FChar &) = default;
    constexpr FChar(FChar &&) = default;
    constexpr FChar &operator=(const FChar &) = default;
    constexpr FChar &operator=(FChar &&) = default;
    ~FChar() = default;

    constexpr FChar(char c, FormatType f = {})
        : c(c)
        , f(f) {}

    constexpr FChar(Utf8Char c)
        : c(c) {}

    constexpr auto size() const {
        return c.size();
    }

    operator std::string_view() const {
        return std::string_view{c};
    }

    operator std::string() const {
        return std::string{std::string_view{*this}};
    }

    friend bool operator==(const FChar &a, const FChar b) {
        return a.c == b.c;
    }

    friend bool operator!=(const FChar &a, const FChar b) {
        return a.c != b.c;
    }

    friend bool operator<(const FChar &a, const FChar b) {
        return a.c < b.c;
    }

    void visit(Archive &arch);
};

// void save(Archive &arch, FChar c);
// void load(Archive &arch, FChar c);
