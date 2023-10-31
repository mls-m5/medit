
#include "text/fstring.h"
#include "core/archive.h"
#include "text/fstringview.h"
#include <algorithm>

FString::FString(const std::string &str, FormatType f)
    : FString(std::string_view{str}, f) {}

FString::FString(std::string_view str, FormatType f) {
    _content.reserve(str.size()); // First guess
    for (size_t i = 0; i < str.size();) {
        auto res = Utf8Char::fromChar(str.data() + i);
        _content.push_back(res.first);
        _content.back().f = f;
        i += res.second ? res.second : 1;
    }
}

FString::FString(FString::const_iterator begin, FString::const_iterator end)
    : _content(begin, end) {}

FString::FString(size_t len, FChar value) {
    _content.resize(len);
    for (auto &c : _content) {
        c = value;
    }
}

FString::FString(const FChar *begin, const FChar *end)
    : _content(begin, end) {}

FString FString::operator+(const FString &other) {
    FString str;
    str.reserve(size() + other.size());

    str._content.insert(str._content.begin(), _content.begin(), _content.end());
    str._content.insert(str._content.end(), other.begin(), other.end());

    return str;
}

bool FString::operator!=(const FString &other) const {
    return _content != other._content;
}

bool FString::operator==(const FString &other) const {
    return _content == other._content;
}

void FString::visit(Archive &arch) {
    arch("str", _content);
}

FString::operator std::string() const {
    std::string str;
    str.reserve(size());
    for (const auto c : _content) {
        for (size_t i = 0; i < c.size(); ++i) {
            str.push_back(c.c[i]);
        }
    }

    return str;
}

size_t FString::find(Utf8Char c, size_t start) const {
    for (size_t i = start; i < size(); ++i) {
        if (at(i) == c) {
            return i;
        }
    }

    return npos;
}

bool FString::operator<(const FString &other) const {
    auto size = std::min(this->size(), other.size());
    for (size_t i = 0; i < size; ++i) {
        auto c1 = static_cast<uint32_t>(at(i).c);
        auto c2 = static_cast<uint32_t>(other.at(i).c);

        if (c1 < c2) {
            return true;
        }
        else if (c1 > c2) {
            return false;
        }
    }

    return this->size() < other.size();
}

FStringView FString::substr(size_t start, size_t length) const {
    if (start >= _content.size()) {
        return {};
    }

    length = std::min(_content.size() - start, length);

    return FStringView{_content.data() + start, length};
}

bool FString::startsWith(FStringView str) {
    if (size() < str.size()) {
        return false;
    }

    for (size_t i = 0; i < str.size(); ++i) {
        if (str.at(i) != at(i)) {
            return false;
        }
    }

    return true;
}

std::vector<FString> FString::split(Utf8Char c) const {
    auto ret = std::vector<FString>{};

    auto first = _content.begin();

    auto it = decltype(first){};

    for (; it = std::find(first, _content.end(), c), it != _content.end();
         first = it, ++first) {
        ret.push_back({first, it});
    }

    ret.push_back({first, _content.end()});

    return ret;
}

FString FString::join(std::vector<FString> strings, Utf8Char c) {

    size_t size = strings.size() - 1;

    for (auto &line : strings) {
        size += line.size();
    }

    auto ret = FString{};
    ret.resize(size, c);

    size_t i = 0;
    for (auto &line : strings) {
        for (auto c : line) {
            ret.at(i) = c;
            ++i;
        }
        ++i;
    }

    return ret;
}
