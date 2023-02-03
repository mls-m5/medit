#pragma once

#include "keys/event.h"

class IInput {
public:
    virtual Event getInput() = 0;

    virtual ~IInput() = default;
};
