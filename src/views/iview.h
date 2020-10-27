#pragma once

#include "meditfwd.h"

class IView {
public:
    virtual void draw(IScreen &window) = 0;

    virtual ~IView() = default;
};
