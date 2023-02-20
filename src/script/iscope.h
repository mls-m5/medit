#pragma once

#include "ienvironment.h"
#include "script/command.h"
#include "script/variable.h"
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

// Scope is used to handle stack depth in scripting
class IScope : public std::enable_shared_from_this<IScope> {
public:
    using Action = std::function<void(std::shared_ptr<IScope>)>;

    //! Return the editor active in the current context
    [[nodiscard]] virtual Editor &editor() = 0;
    [[nodiscard]] virtual IEnvironment &env() = 0;

    //! @returns parent environment
    [[nodiscard]] virtual IScope &parent() = 0;

    [[nodiscard]] virtual IScope &root() = 0;
    [[nodiscard]] const virtual IScope &root() const = 0;

    virtual void addCommand(std::string,
                            std::function<void(std::shared_ptr<IScope>)>) = 0;

    //! Run a single command, return true on success and false on fail
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

    virtual Action *findAction(const std::string &name) = 0;

    virtual void set(std::string name, Variable variable) = 0;
    virtual std::optional<Variable> get(std::string name) const = 0;

    IScope() = default;
    IScope(const IScope &) = delete;
    IScope &operator=(const IScope &) = delete;
    IScope(IScope &&) = delete;
    IScope &operator=(IScope &&) = delete;

    virtual ~IScope() = default;
};
