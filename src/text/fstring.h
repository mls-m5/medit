#pragma once

#include "fchar.h"
#include <vector>

class FString {
public:
    FString() = default;
    FString(const FString &) = default;
    FString(FString &&) = default;
    FString &operator=(const FString &) = default;
    FString &operator=(FString &&) = default;

    // Convert from a standard string to a fstring
    FString(const std::string &str, FormatType f = {});
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

    auto &emplace_back(Utf8Char c) {
        _content.push_back(c);
        return _content.back();
    }

    auto operator+=(const FString &other) {
        _content.insert(_content.end(), other.begin(), other.end());
    }

    std::vector<FChar> _content;
};
