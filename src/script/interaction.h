#pragma once

#include "text/fstring.h"
#include "text/position.h"
#include <algorithm>
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

/// Simple interaciton variant with colon separated keys etc
struct SimpleInteraction {
    std::string op;
    std::vector<std::pair<std::string, std::string>> values;
    bool valid = true;

    /// Create text form
    void serialize(std::ostream &os) const;

    std::string serialize() const;

    /// Parse text form
    void deserialize(std::istream &is);

    void deserialize(const std::string &in);

    std::string_view at(std::string_view name) const {
        for (auto &it : values) {
            if (it.first == name) {
                return it.second;
            }
        }

        return {};
    }
};

struct Interaction {
    FString text;
    Position cursorPosition;
};
