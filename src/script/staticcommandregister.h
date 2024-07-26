#pragma once

#include "script/ienvironment.h"
#include <functional>
#include <string>

class StaticCommandRegister {
public:
    using FunctionType = std::function<void(std::shared_ptr<IEnvironment>)>;

    struct Entity {
        std::string name;
        FunctionType function;
    };

    StaticCommandRegister(std::string name, FunctionType function);
};
