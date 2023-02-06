#pragma once

#include "nlohmann/json.hpp"
#include "text/utf8char.h"

inline void to_json(nlohmann::json &j, const Utf8Char &c) {
    j = std::string{c.begin(), c.end()};
}

inline void from_json(const nlohmann::json &j, Utf8Char &c) {
    auto s = std::string{j};
    c.fromChar(s.data());
}
