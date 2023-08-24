#pragma once

#include "cursor.h"
#include "fchar.h"
#include "fstring.h"
#include <algorithm>
#include <stdexcept>
#include <string_view>

class FStringView {
    const FChar *_data = nullptr;
    size_t _size = 0;

public:
    FStringView(const FString &str);

    constexpr FStringView(const FChar *data, size_t size)
        : _data{data}
        , _size{size} {}

    /// Only supports single line ranges
    FStringView(const Cursor begin, Cursor end);

    FStringView() = default;
    FStringView(const FStringView &) = default;
    FStringView(FStringView &&) = default;
    FStringView &operator=(const FStringView &) = default;
    FStringView &operator=(FStringView &&) = default;
    ~FStringView() = default;

    constexpr const FChar *data() const {
        return _data;
    }

    constexpr size_t size() const {
        return _size;
    }

    constexpr const FChar *begin() const {
        return _data;
    }

    constexpr const FChar *end() const {
        return _data + _size;
    }

    bool operator==(const FStringView &other) const {
        return std::equal(begin(), end(), other.begin(), other.end());
    }

    bool operator!=(const FStringView &other) const {
        return !std::equal(begin(), end(), other.begin(), other.end());
    }

    bool operator==(const FString &other) const {
        return *this == FStringView{other};
    }

    bool operator!=(const FString &other) const {
        return *this != FStringView{other};
    }

    constexpr const FChar &at(size_t i) const {
        if (i > _size) {
            throw std::out_of_range{"trying to access out of range element of "
                                    "FStringView " __FILE__};
        }
        return _data[i];
    }

    constexpr const FChar &front() const {
        if (!_size) {
            throw std::out_of_range{"trying to access front of empty "
                                    "FStringView " __FILE__};
        }

        return *_data;
    }

    constexpr const FChar &back() const {
        if (!_size) {
            throw std::out_of_range{"trying to access back() of empty "
                                    "FStringView " __FILE__};
        }

        return *(_data + _size - 1);
    }

    constexpr bool empty() const {
        return !_size;
    }

    explicit operator std::string() const {
        std::string str;
        str.reserve(size());
        for (const auto c : *this) {
            for (size_t i = 0; i < c.size(); ++i) {
                str.push_back(c.c[i]);
            }
        }

        return str;
    }

    explicit operator FString() const;
};

inline bool operator==(const FString &a, const FStringView &b) {
    return b == FStringView{a};
}

inline bool operator!=(const FString &a, const FStringView &b) {
    return b != FStringView{a};
}

inline std::ostream &operator<<(std::ostream &out, const FStringView &str) {
    out << std::string{str};
    return out;
}

void save(Archive &arch, FStringView &value);

inline void load(Archive &arch, FStringView &value) {
    static_assert("not supportoed");
}
