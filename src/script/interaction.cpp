#include "interaction.h"
#include <functional>
#include <memory>
#include <sstream>

void SimpleInteraction::serialize(std::ostream &os) const {
    if (!valid) {
        return;
    }

    os << op << "\n";
    for (const auto &[key, value] : values) {
        os << key << ": " << value << "\n";
    }
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
