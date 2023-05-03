#pragma once

#include "fstring_serialization.h"
#include "fstringview.h"

inline void to_json(nlohmann::json &j, const FStringView &c) {
    j = FString{c};
    //    j = nlohmann::json{c.begin(), c.end()};
}

// From json is not allowed
