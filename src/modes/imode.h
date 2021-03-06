#pragma once

#include "meditfwd.h"
#include "screen/cursorstyle.h"
#include <functional>
#include <memory>
#include <string_view>

class IMode {
public:
    virtual bool keyPress(std::shared_ptr<IEnvironment>) = 0;
    virtual std::string_view name() const = 0;

    virtual void start(Editor &) = 0;
    virtual void exit(Editor &) = 0;
    virtual CursorStyle cursorStyle() const = 0;

    virtual ~IMode() = default;
};
