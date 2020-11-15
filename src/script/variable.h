#pragma once

#include <string>

class Variable {
    std::string _value;

public:
    Variable(std::string value) : _value(value) {}

    Variable() = default;
    Variable(const Variable &) = default;
    Variable(Variable &&) = default;
    Variable &operator=(const Variable &) = default;
    Variable &operator=(Variable &&) = default;

    const std::string &value() const {
        return _value;
    }
};
