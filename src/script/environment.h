#pragma once

#include "ienvironment.h"

class Environment : public IEnvironment {
    Editor *_editor = nullptr;
    KeyEvent _lastKeyEvent;

public:
    // Used by handler
    void editor(Editor *editor) {
        _editor = editor;
    }

    void key(KeyEvent e) {
        _lastKeyEvent = e;
    }

    Editor &editor() override {
        if (!_editor) {
            throw std::runtime_error("trying to use editor when not set");
        }
        return *_editor;
    }

    KeyEvent key() override {
        return _lastKeyEvent;
    }
};
