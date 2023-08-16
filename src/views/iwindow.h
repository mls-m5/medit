#pragma once

#include "keys/ikeysink.h"
#include "views/iview.h"

class IWindow : public virtual IView, public virtual IKeySink {
public:
    virtual bool isClosed() = 0;
    virtual void close() = 0;

    virtual void triggerRedraw() = 0;

    virtual IEnvironment &env() = 0;

    //! @return Focused editor if present or nullptr if window does not have one
    virtual Editor *currentEditor() = 0;
};
