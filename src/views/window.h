#pragma once

#include "keys/ikeysink.h"
#include "views/iwindow.h"
#include "views/view.h"

class Window : public virtual View, public virtual IWindow {
    bool _isClosed = false;

public:
    bool isClosed() override {
        return _isClosed;
    }

    void close() override {
        _isClosed = true;
    }
};
