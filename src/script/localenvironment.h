#pragma once

#include "core/registers.h"
#include "ienvironment.h"
#include "keys/event.h"
#include "meditfwd.h"
#include "script/luastate.h"
#include "script/standardcommands.h"
#include "views/editor.h"

/// Environment data for a single user
class LocalEnvironment : public IEnvironment {
    MainWindow &_mainWindow;
    KeyEvent _lastKeyEvent;
    Editor *_console;
    Project *_project;
    bool _showConsole = false;
    Context &_context;
    Registers _registers;
    LuaState _lua;

public:
    LocalEnvironment(MainWindow &mw, Context &context);

    void initLua(MainWindow &mw);

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

    MainWindow &mainWindow() override {
        return _mainWindow;
    }

    const StandardCommands &standardCommands() const override;

    //    void parseLua(std::string_view code);

    //! Trigger redraw of winow
    void redrawScreen();
};
