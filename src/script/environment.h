#pragma once

#include "ienvironment.h"

class Environment : public IEnvironment {
    Editor *_editor = nullptr;
    KeyEvent _lastKeyEvent;
    Buffer *_console;
    bool _showConsole = false;

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

    void console(Buffer *console) {
        _console = console;
    }

    KeyEvent key() const override {
        return _lastKeyEvent;
    }

    Buffer &console() override {
        if (!_console) {
            throw std::runtime_error("no console set");
        }
        return *_console;
    }

    void showConsole(bool shown) override {
        _showConsole = shown;
    }

    bool showConsole() {
        return _showConsole;
    }
};
