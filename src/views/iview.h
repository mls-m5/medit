#pragma once

#include "meditfwd.h"
#include <cstddef>

class IView {
public:
    virtual void draw(IScreen &window) = 0;

    virtual ~IView() = default;

    virtual size_t x() const = 0;
    virtual size_t y() const = 0;
    virtual size_t width() const = 0;
    virtual size_t height() const = 0;
};
