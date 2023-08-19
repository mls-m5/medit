#pragma once

#include "meditfwd.h"
#include "text/position.h"
#include <filesystem>
#include <functional>
#include <memory>

class IRename {
public:
    IRename() = default;
    IRename(const IRename &) = delete;
    IRename(IRename &&) = delete;
    IRename &operator=(const IRename &) = delete;
    IRename &operator=(IRename &&) = delete;
    virtual ~IRename() = default;

    struct RenameArgs {
        std::string newName;
    };

    struct PrepareCallbackArgs {
        Position start;
        Position end;
    };

    virtual bool doesSupportPrepapre() = 0;

    virtual bool prepare(std::shared_ptr<IEnvironment> env,
                         std::function<void(PrepareCallbackArgs)>) = 0;

    virtual bool rename(std::shared_ptr<IEnvironment>,
                        RenameArgs,
                        std::function<void(const Changes &)> callback) = 0;

    virtual bool shouldEnable(std::filesystem::path) const = 0;

    virtual int priority() const = 0;
};
