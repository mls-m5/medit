#pragma once

#include "keys/keyevent.h"
#include "meditfwd.h"

class IMode {
public:
    virtual void keyPress(const KeyEvent &, Editor &) = 0;
    virtual std::string_view name() = 0;

    virtual ~IMode() = default;
};
