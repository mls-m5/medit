#pragma once

#include "meditfwd.h"

// Things common to the whole application
class IEnvironment {
public:
    //! @returns last key pressed
    [[nodiscard]] virtual KeyEvent key() const = 0;

    [[nodiscard]] virtual Registers &registers() = 0;

    [[nodiscard]] virtual Project &project() = 0;

    [[nodiscard]] virtual Context &context() = 0;

    [[nodiscard]] virtual CoreEnvironment &core() = 0;

    //! Return the editor active in the current context
    //    [[nodiscard]] virtual Editor &editor() = 0;
    [[nodiscard]] virtual Editor &console() = 0;

    //! In the future this can be multiple values depending on which console
    virtual void showConsole(bool shown) = 0;

    IEnvironment() = default;
    IEnvironment(const IEnvironment &) = delete;
    IEnvironment &operator=(const IEnvironment &) = delete;
    IEnvironment(IEnvironment &&) = delete;
    IEnvironment &operator=(IEnvironment &&) = delete;

    virtual ~IEnvironment() = default;
};
