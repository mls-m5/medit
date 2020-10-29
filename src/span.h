#pragma once

#include <cstddef>

//! Until i get c++20
template <class T>
class span {
public:
    constexpr span() = default;
    constexpr span(const span &) = default;
    constexpr span(span &&) = default;
    constexpr span &operator=(const span &) = default;
    constexpr span &operator=(span &&) = default;

    constexpr span(T *begin, T *end) : _begin(begin), _end(end) {}
    constexpr span(T *begin, size_t size) : _begin(begin), _end(begin + size) {}

    constexpr auto begin() {
        return _begin;
    }

    constexpr auto end() {
        return _end;
    }

    constexpr size_t size() {
        return _end - _begin;
    }

    constexpr auto byte_size() {
        return size() * sizeof(T);
    }

private:
    T *_begin;
    T *_end;
};
