
#include "text/fstring.h"

FString::FString(const std::string &str, FormatType f)
    : FString(std::string_view{str}, f) {}

FString::FString(std::string_view str, FormatType f) {
    _content.reserve(str.size()); // First guess
    for (size_t i = 0; i < str.size();) {
        auto res = Utf8Char::fromChar(str.data() + i);
        _content.push_back(res.first);
        _content.back().f = f;
        i += res.second;
    }
}

FString::FString(FString::const_iterator begin, FString::const_iterator end)
    : _content(begin, end) {}

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
