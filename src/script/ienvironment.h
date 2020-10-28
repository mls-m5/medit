#pragma once

#include "keys/keyevent.h"
#include "views/editor.h"

class IEnvironment {
public:
    virtual Editor &editor() = 0;
    virtual KeyEvent key() = 0;
};
