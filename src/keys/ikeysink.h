#pragma once

#include "meditfwd.h"

class IKeySink {
public:
    //! @return true if key is handled or false if key should be handled
    //! by somebody else
    virtual bool keyPress(IEnvironment &env) = 0;

    //! Move the position of the cursor. Done after everything is rendered
    virtual void updateCursor(IScreen &screen) const = 0;

    virtual ~IKeySink() = default;
};
