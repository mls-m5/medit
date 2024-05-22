#pragma once

#include "text/fstring.h"
#include <istream>
#include <string>
#include <string_view>
#include <vector>

/// Simple interaciton variant with colon separated keys etc
/// Can be used as an example for implementing own more advanced interactions
struct SimpleInteraction {
    std::string op;
    std::vector<std::pair<std::string, std::string>> values;
    std::string title;
    bool valid = true;

    /// Create text form
    FString serialize() const;

    /// Parse text form
    void deserialize(std::istream &is);

    void deserialize(const std::string &in);

    /// Get reference for a name
    std::string_view at(std::string_view name) const;
};
