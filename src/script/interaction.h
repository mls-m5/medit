#pragma once

#include "text/position.h"
#include <algorithm>
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

struct Interaction {
    std::string op;
    std::vector<std::pair<std::string, std::string>> values;
    Position cursorPosition;

    bool valid = true;

    /// Create text form
    void serialize(std::ostream &os) const;

    /// Parse text form
    void deserialize(std::istream &is);

    std::string_view at(std::string_view name) const {
        for (auto &it : values) {
            if (it.first == name) {
                return it.second;
            }
        }

        return {};
    }
};
