#pragma once

#include "meditfwd.h"

#include "text/fstringview.h"
#include <functional>
#include <memory>

// Things common to the whole application
class IEnvironment : public std::enable_shared_from_this<IEnvironment> {
public:
    //! @returns last key pressed
    [[nodiscard]] virtual KeyEvent key() const = 0;

    [[nodiscard]] virtual Registers &registers() = 0;

    [[nodiscard]] virtual Project &project() = 0;

    [[nodiscard]] virtual ThreadContext &context() = 0;

    [[nodiscard]] virtual CoreEnvironment &core() = 0;

    [[nodiscard]] virtual MainWindow &mainWindow() = 0;

    //! Return the editor active in the current context
    [[nodiscard]] virtual Editor &editor() = 0;
    [[nodiscard]] virtual Editor &console() = 0;

    [[nodiscard]] virtual InteractionHandling &interactions() = 0;

    virtual void statusMessage(FStringView string) = 0;

    //! In the future this can be multiple values depending on which console
    virtual void showConsole(bool shown = true) = 0;

    virtual const StandardCommands &standardCommands() const = 0;

    virtual void guiTask(std::function<void()> f) = 0;

    IEnvironment() = default;
    IEnvironment(const IEnvironment &) = delete;
    IEnvironment &operator=(const IEnvironment &) = delete;
    IEnvironment(IEnvironment &&) = delete;
    IEnvironment &operator=(IEnvironment &&) = delete;

    virtual ~IEnvironment() = default;
};
