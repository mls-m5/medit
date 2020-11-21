#include "core/registers.h"

void Registers::save(char registerName, std::string str, bool isLine) {
    _registers[registerName] = {str, isLine};
}

Registers::Register Registers::load(char registerName) const {
    if (auto f = _registers.find(registerName); f != _registers.end()) {
        return f->second;
    }
    else {
        return {""};
    }
}
