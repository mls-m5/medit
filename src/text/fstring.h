#pragma once

#include "fchar.h"
#include <vector>

class FString {
    std::vector<FChar> _content;

public:
    using iterator = std::vector<FChar>::iterator;
    using const_iterator = std::vector<FChar>::const_iterator;

    FString() = default;
    FString(const FString &) = default;
    FString(FString &&) = default;
    FString &operator=(const FString &) = default;
    FString &operator=(FString &&) = default;

    // Convert from a standard string to a fstring
    FString(const std::string &str, FormatType f = 1);
    FString(std::string_view str, FormatType f = 1);
    FString(const_iterator begin, const_iterator end);
    FString(const char *str, FormatType f = 1)
        : FString(std::string_view(str), f) {}
    FString(size_t len, FChar c);
    operator std::string() const;

    [[nodiscard]] auto size() const {
        return _content.size();
    }

    [[nodiscard]] auto empty() const {
        return _content.empty();
    }

    [[nodiscard]] auto begin() const {
        return _content.begin();
    }

    [[nodiscard]] auto begin() {
        return _content.begin();
    }

    [[nodiscard]] auto end() const {
        return _content.end();
    }

    [[nodiscard]] auto end() {
        return _content.end();
    }

    [[nodiscard]] auto at(size_t index) const {
        return _content.at(index);
    }

    [[nodiscard]] auto &at(size_t index) {
        return _content.at(index);
    }

    [[nodiscard]] auto front() const {
        return _content.front();
    }

    [[nodiscard]] auto back() const {
        return _content.back();
    }

    auto erase(size_t pos, size_t size) {
        return _content.erase(_content.begin() + pos,
                              _content.begin() + (pos + size));
    }

    auto insert(size_t pos, FChar value) {
        return _content.insert(_content.begin() + pos, value);
    }

    auto insert(std::vector<FChar>::iterator place, FChar value) {
        return _content.insert(place, value);
    }

    auto insert(iterator position, iterator begin, iterator end) {
        return _content.insert(position, begin, end);
    }

    auto insert(iterator position, FString string) {
        insert(position, string.begin(), string.end());
    }

    auto &emplace_back(Utf8Char c) {
        _content.push_back(c);
        return _content.back();
    }

    auto erase(iterator begin, iterator end) {
        return _content.erase(begin, end);
    }

    auto erase(iterator it) {
        return _content.erase(it);
    }

    void resize(size_t size, FChar fill) {
        _content.resize(size, fill);
    }

    auto operator+=(const FString &other) {
        _content.insert(_content.end(), other.begin(), other.end());
    }

    void reserve(size_t size) {
        _content.reserve(size);
    }

    void clear() {
        _content.clear();
    }

    void format(FormatType f) {
        for (auto &c : _content) {
            c.f = f;
        }
    }

    FString operator+(const FString &other);
};
