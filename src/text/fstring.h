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
    FString(const std::string &str);
    operator std::string();

    [[nodiscard]] auto size() const {
        return _content.size();
    }

    [[nodiscard]] auto empty() const {
        return _content.empty();
    }

    auto erase(size_t pos, size_t size) {
        return _content.erase(_content.begin() + pos,
                              _content.begin() + (pos + size));
    }

    auto &emplace_back(Utf8Char c) {
        _content.push_back(c);
        return _content.back();
    }

    std::vector<FChar> _content;
};
