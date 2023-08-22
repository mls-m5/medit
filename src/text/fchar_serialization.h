#pragma once

#pragma once

#include "nlohmann/json.hpp"
#include "text/fchar.h"

inline void to_json(nlohmann::json &j, const FChar &c) {
    auto data = std::array<int, 5>{};
    std::copy(c.c.begin(), c.c.end(), data.begin());
    data[4] = c.f;
    j["c"] = data;
}

inline void from_json(const nlohmann::json &j, FChar &c) {
    auto data = std::array<int, 5>{};
    data = j.at("c");
    std::copy(data.begin(), data.end(), c.c.begin());
    c.f = data.at(4);
}
