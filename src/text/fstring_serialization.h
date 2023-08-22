#pragma once

#include "fchar_serialization.h"
#include "fstring.h"
#include "nlohmann/json.hpp"

inline void to_json(nlohmann::json &j, const FString &c) {
    j["str"] = c.content();
}

inline void from_json(const nlohmann::json &j, FString &c) {
    c.content() = j["str"];
}
