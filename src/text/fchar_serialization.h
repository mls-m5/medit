#pragma once

#pragma once

#include "nlohmann/json.hpp"
#include "text/fchar.h"

// inline void to_json(nlohmann::json &j, const FChar &c) {
//     auto str = std::string{c.c.begin(), c.c.end()};
//     str.push_back(static_cast<char>(c.f));
//     j = str;
// }

// inline void from_json(const nlohmann::json &j, FChar &c) {
//     auto s = std::string{j};
//     std::copy(s.begin(), s.begin() + 4, c.c.begin());
//     c.f = s.back();
// }

inline void to_json(nlohmann::json &j, const FChar &c) {
    auto data = std::array<int, 5>{};
    std::copy(c.c.begin(), c.c.end(), data.begin());
    data[4] = c.f;
    j = data;
}

inline void from_json(const nlohmann::json &j, FChar &c) {
    auto data = std::array<int, 5>{};
    data = j;
    std::copy(data.begin(), data.end(), c.c.begin());
    c.f = data.at(4);
}
