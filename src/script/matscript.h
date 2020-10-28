#pragma once

#include "ienvironment.h"
#include <string>
#include <string_view>

void run(std::string_view command, IEnvironment &env);

inline void run(const std::string &command, IEnvironment &env) {
    run(std::string_view{command}, env);
}

inline void run(const char *command, IEnvironment &env) {
    run(std::string_view{command}, env);
}
