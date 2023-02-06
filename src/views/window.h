#pragma once

#include "keys/ikeysink.h"
#include "views/iwindow.h"
#include "views/view.h"

class Window : public virtual View, public virtual IWindow {
    IWindow *_parent = nullptr;

    bool _isClosed = false;

public:
    Window(IWindow *parent)
        : _parent{parent} {}

    bool isClosed() override {
        return _isClosed;
    }

    void close() override {
        _isClosed = true;
    }

    void triggerRedraw() override {
        _parent->triggerRedraw();
    }

    IEnvironment &env() override {
        return _parent->env();
    }
};
