#pragma once

#include "text/fstring.h"

class IScreen {
public:
    virtual void draw(size_t x, size_t y, FString str) = 0;
    virtual void refresh() = 0;
    virtual void clear() = 0;
    virtual void cursor(size_t x, size_t y) = 0;

    virtual size_t x() const = 0;
    virtual size_t y() const = 0;
    virtual size_t width() const = 0;
    virtual size_t height() const = 0;
};
