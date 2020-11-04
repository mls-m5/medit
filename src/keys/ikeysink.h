#pragma once

#include "script/ienvironment.h"

class IKeySink {
public:
    virtual void keyPress(IEnvironment &env) = 0;
    virtual void updateCursor(IScreen &screen) const = 0;

    virtual ~IKeySink() = default;
};
