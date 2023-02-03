#pragma once

#include "keys/keyevent.h"

class IInput {
public:
    virtual Event getInput() = 0;

    virtual ~IInput() = default;
};
