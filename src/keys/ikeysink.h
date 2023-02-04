#pragma once

#include "meditfwd.h"
#include <memory>

class IKeySink {
public:
    //! @return true if key is handled or false if key should be handled
    //! by somebody else
    virtual bool keyPress(std::shared_ptr<IScope> env) = 0;

    //! Move the position of the cursor. Done after everything is rendered
    virtual void updateCursor(IScreen &screen) const = 0;

    virtual ~IKeySink() = default;

    //! @return true if key is handled
    virtual bool mouseDown(int x, int y) = 0;
};
