#pragma once

#include "meditfwd.h"

class IView {
    virtual void draw(IScreen &window) = 0;
};
