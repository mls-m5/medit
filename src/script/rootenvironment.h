#pragma once

#include "environment.h"
#include "meditfwd.h"
#include "syntax/palette.h"

class RootEnvironment : public Environment {
    KeyEvent _lastKeyEvent;
    Editor *_console;
    Project *_project;
    bool _showConsole = false;
    Palette _palette;

public:
    RootEnvironment() : Environment(nullptr) {}

    void key(KeyEvent e) {
        _lastKeyEvent = e;
    }

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

    void project(Project *project) {
        _project = project;
    }

    //! @see IEnvironment
    Palette &palette() override {
        return _palette;
    }

    Project &project() override {
        if (!_project) {
            throw std::runtime_error("no project defined");
        }
        return *_project;
    }

    void call(std::string value) {
        if (value == "showlocator") {
        }
    }
};
