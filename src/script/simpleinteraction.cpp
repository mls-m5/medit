#include "simpleinteraction.h"

#include "syntax/palette.h"
#include "text/fstring.h"
#include <functional>
#include <memory>
#include <sstream>

FString SimpleInteraction::serialize() const {
    if (!valid) {
        return {};
    }

    auto text = FString{op};
    text += "\n";

    for (const auto &[key, value] : values) {
        text += FString{key, Palette::identifier} += ": ";
        text += FString{value} += "\n";
    }

    return text;
}

void SimpleInteraction::deserialize(std::istream &is) {
    valid = false;
    values.clear();

    std::string line;
    if (!std::getline(is, line)) {
        valid = false;
        return;
    }

    {
        if (size_t commentStart = line.find('#');
            commentStart != std::string::npos) {
            line = line.substr(0, commentStart);
        }
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace),
                   line.end());
    }

    op = line;

    while (std::getline(is, line)) {
        if (auto commentStart = line.find('#');
            commentStart != std::string::npos) {
            line = line.substr(0, commentStart);
        }

        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);

        // TODO: This removes all spaces. This might be okay for renaming
        // but might need refactorization later on
        key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
        value.erase(std::remove_if(value.begin(), value.end(), ::isspace),
                    value.end());
        values.emplace_back(key, value);
    }

    valid = !op.empty() && !values.empty();
}

void SimpleInteraction::deserialize(const std::string &in) {
    auto ss = std::istringstream{in};
    deserialize(ss);
}

std::string_view SimpleInteraction::at(std::string_view name) const {
    for (auto &it : values) {
        if (it.first == name) {
            return it.second;
        }
    }

    return {};
}
