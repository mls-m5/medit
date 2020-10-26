#pragma once

#include "keys/keyevent.h"

class IInput {
    virtual KeyEvent getInput() = 0;
};
