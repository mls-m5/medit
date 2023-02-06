#pragma once

#include "nlohmann/json.hpp"
#include "text/utf8char.h"

inline void to_json(nlohmann::json &j, const Utf8Char &c) {
    auto data = std::array<int, 4>{};
    std::copy(c.begin(), c.end(), data.begin());
    j = data;
}

inline void from_json(const nlohmann::json &j, Utf8Char &c) {
    auto data = std::array<int, 4>{};
    data = j;
    std::copy(data.begin(), data.end(), c.begin());
}
