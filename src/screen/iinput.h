#pragma once

#include "keys/keyevent.h"

class IInput {
public:
    virtual KeyEvent getInput() = 0;
};
