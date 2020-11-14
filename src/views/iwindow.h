#pragma once

#include "keys/ikeysink.h"
#include "views/view.h"

class IWindow : public virtual IView, public virtual IKeySink {
public:
    virtual bool isClosed() = 0;
    virtual void close() = 0;
};
