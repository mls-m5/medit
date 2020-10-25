#pragma once

#include "keyevent.h"

class IMode {
public:
    virtual int keyPress(KeyEvent c) = 0;
};
