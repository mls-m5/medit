#pragma once

#include "environment.h"

class RootEnvironment : public Environment {
    KeyEvent _lastKeyEvent;
    Editor *_console;
    bool _showConsole = false;

public:
    RootEnvironment() : Environment(nullptr) {}

    void key(KeyEvent e) {
        _lastKeyEvent = e;
    }

    //    Editor &editor() override {
    //        if (!_editor) {
    //            throw std::runtime_error("trying to use editor when not set");
    //        }
    //        return *_editor;
    //    }

    void console(Editor *console) {
        _console = console;
    }

    KeyEvent key() const override {
        return _lastKeyEvent;
    }

    Editor &console() override {
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

    IEnvironment &parent() override {
        throw std::runtime_error("trying to access parent of root environment");
    }

    IEnvironment &root() override {
        return *this;
    }

    const IEnvironment &root() const override {
        return *this;
    }

    void call(std::string value) {
        if (value == "showlocator") {
        }
    }
};
