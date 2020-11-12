#pragma once

#include "keys/keyevent.h"
#include "meditfwd.h"
#include "script/command.h"
#include <functional>
#include <string>

class IEnvironment {
public:
    //! Return the editor active in the current context
    [[nodiscard]] virtual Editor &editor() = 0;
    [[nodiscard]] virtual Editor &console() = 0;

    //! @returns last key pressed
    [[nodiscard]] virtual KeyEvent key() const = 0;

    //! @returns parent environment, or nullptr if no parent present
    [[nodiscard]] virtual IEnvironment &parent() = 0;

    [[nodiscard]] virtual IEnvironment &root() = 0;
    [[nodiscard]] const virtual IEnvironment &root() const = 0;

    virtual void addCommand(std::string,
                            std::function<void(IEnvironment &)>) = 0;

    //! In the future this can be multiple values depending on which console
    virtual void showConsole(bool shown) = 0;

    //! Run a single command, returne true on success and false on fail
    virtual bool run(const Command &command) = 0;

    inline bool run(const CommandBlock &commands) {
        if (commands._command && !run(commands._command)) {
            return false;
        }
        for (auto &command : commands) {
            if (!run(command)) {
                return false;
            }
        }
        return true;
    }

    IEnvironment() = default;
    IEnvironment(const IEnvironment &) = delete;
    IEnvironment &operator=(const IEnvironment &) = delete;
    IEnvironment(IEnvironment &&) = delete;
    IEnvironment &operator=(IEnvironment &&) = delete;

    virtual ~IEnvironment() = default;
};
