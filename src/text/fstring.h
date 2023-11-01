#pragma once

#include "fchar.h"
#include "syntax/palette.h"
#include "text/utf8char.h"
#include <ostream>
#include <vector>

class FStringView;

/// Formatted string; string with formatted characters
class FString {
public:
    using VectorT = std::vector<FChar>;
    using iterator = VectorT::iterator;
    using const_iterator = VectorT::const_iterator;

    static constexpr size_t npos = static_cast<size_t>(-1);

    FString() = default;
    FString(const FString &) = default;
    FString(FString &&) = default;
    FString &operator=(const FString &) = default;
    FString &operator=(FString &&) = default;
    ~FString() = default;

    // Convert from a standard string to a fstring
    FString(const std::string &str, FormatType f = 1);
    FString(std::string_view str, FormatType f = 1);
    FString(const_iterator begin, const_iterator end);
    FString(const char *str, FormatType f = 1)
        : FString(std::string_view(str), f) {}
    FString(size_t len, FChar c);
    FString(const FChar *begin, const FChar *end);
    operator std::string() const;

    [[nodiscard]] size_t size() const {
        return _content.size();
    }

    [[nodiscard]] bool empty() const {
        return _content.empty();
    }

    [[nodiscard]] VectorT::const_iterator begin() const {
        return _content.begin();
    }

    [[nodiscard]] VectorT::iterator begin() {
        return _content.begin();
    }

    [[nodiscard]] VectorT::const_iterator end() const {
        return _content.end();
    }

    [[nodiscard]] VectorT::iterator end() {
        return _content.end();
    }

    [[nodiscard]] const FChar &at(size_t index) const {
        return _content.at(index);
    }

    [[nodiscard]] FChar &at(size_t index) {
        return _content.at(index);
    }

    [[nodiscard]] FChar front() const {
        return _content.front();
    }

    [[nodiscard]] FChar back() const {
        return _content.back();
    }

    [[nodiscard]] size_t find(Utf8Char c, size_t start = 0) const;

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

    auto pop_back() {
        _content.pop_back();
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

    FString &operator+=(const FString &other) {
        _content.insert(_content.end(), other.begin(), other.end());
        return *this;
    }

    bool operator<(const FString &other) const;
    bool operator<(const FStringView &other) const;
    friend bool operator<(const FStringView &lhs, const FString &rhs);

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

    FChar *data() {
        return _content.data();
    }

    const FChar *data() const {
        return _content.data();
    }

    VectorT &content() {
        return _content;
    }

    const auto &content() const {
        return _content;
    }

    FStringView substr(size_t start, size_t length = size_t(-1)) const;

    bool startsWith(FStringView str);

    std::vector<FString> split(Utf8Char c = '\n') const;

    static FString join(std::vector<FString> strings, Utf8Char c = '\n');

    FString operator+(const FString &other);

    bool operator!=(const FString &other) const;
    bool operator==(const FString &other) const;

    void visit(Archive &arch);

private:
    VectorT _content;
};

inline std::ostream &operator<<(std::ostream &out, const FString &str) {
    out << std::string{str};
    return out;
}
