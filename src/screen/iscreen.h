#pragma once

#include "text/fstring.h"

class IScreen {
public:
    virtual void draw(size_t x, size_t y, FString str) = 0;
    virtual void refresh() = 0;
};
