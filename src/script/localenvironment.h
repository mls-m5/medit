#pragma once

#include "core/registers.h"
#include "ienvironment.h"
#include "keys/event.h"
#include "meditfwd.h"

/// Environment data for a single user
class LocalEnvironment : public IEnvironment {
    KeyEvent _lastKeyEvent;
    Editor *_console;
    Project *_project;
    bool _showConsole = false;
    Context &_context;
    Registers _registers;

public:
    LocalEnvironment(Context &context) : _context{context} {}

    void key(KeyEvent e) {
        _lastKeyEvent = e;
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

    void console(Editor *console) {
        _console = console;
    }

    void showConsole(bool shown) override {
        _showConsole = shown;
    }

    bool showConsole() {
        return _showConsole;
    }

    void project(Project *project) {
        _project = project;
    }

    Context &context() override {
        return _context;
    }

    Registers &registers() override {
        return _registers;
    }

    Project &project() override {
        if (!_project) {
            throw std::runtime_error("no project defined");
        }
        return *_project;
    }

    CoreEnvironment &core() override;
};
