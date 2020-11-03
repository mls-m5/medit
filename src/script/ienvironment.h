#pragma once

#include "keys/keyevent.h"
#include "views/editor.h"

class IEnvironment {
public:
    [[nodiscard]] virtual Editor &editor() = 0;
    [[nodiscard]] virtual Buffer &console() = 0;
    [[nodiscard]] virtual KeyEvent key() const = 0;

    virtual ~IEnvironment() = default;
};
