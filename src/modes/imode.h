#pragma once

#include "meditfwd.h"
#include <functional>
#include <string_view>

class IMode {
public:
    virtual bool keyPress(IEnvironment &env) = 0;
    virtual std::string_view name() = 0;

    //    virtual void startCallback(std::function<void(Editor &)> f) = 0;
    //    virtual void exitCallback(std::function<void(Editor &)> f) = 0;

    virtual void start(Editor &) = 0;
    virtual void exit(Editor &) = 0;

    virtual ~IMode() = default;
};
