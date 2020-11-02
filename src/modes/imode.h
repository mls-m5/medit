#pragma once

#include "meditfwd.h"
#include <string_view>

class IMode {
public:
    virtual bool keyPress(IEnvironment &env, bool useDefault = true) = 0;
    virtual std::string_view name() = 0;

    virtual ~IMode() = default;
};
