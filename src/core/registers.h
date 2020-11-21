#pragma once

#include <map>
#include <string>

constexpr char standardRegister = '"';

class Registers {
public:
    struct Register {
        std::string value;
        bool isLine = false;

        operator std::string &() {
            return value;
        }
    };

    void save(char registerName, std::string str, bool isLine = false);
    Register load(char registerName) const;

private:
    std::map<char, Register> _registers;
};
