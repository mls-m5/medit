#pragma once

#include "meditfwd.h"
#include <string_view>

class IMode {
public:
    virtual void keyPress(IEnvironment &) = 0;
    virtual std::string_view name() = 0;

    virtual ~IMode() = default;
};
