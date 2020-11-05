#pragma once

#include "keys/keyevent.h"
#include "meditfwd.h"

class IEnvironment {
public:
    [[nodiscard]] virtual Editor &editor() = 0;
    [[nodiscard]] virtual Editor &console() = 0;

    //! @returns last key pressed
    [[nodiscard]] virtual KeyEvent key() const = 0;

    //! @returns parent environment, or nullptr if no parent present
    [[nodiscard]] virtual IEnvironment *parent() = 0;

    //! In the future this can be multiple values depending on which console
    virtual void showConsole(bool shown) = 0;

    virtual ~IEnvironment() = default;
};
