#pragma once

#include "core/registers.h"
#include "ienvironment.h"
#include "keys/event.h"
#include "meditfwd.h"
#include "script/standardcommands.h"
#include "views/editor.h"
#include <functional>

/// Environment data for a single user
class LocalEnvironment : public IEnvironment {
    CoreEnvironment &_core;
    MainWindow &_mainWindow;
    KeyEvent _lastKeyEvent;
    Editor *_console = nullptr;
    bool _showConsole = false;
    ThreadContext &_context;
    Registers _registers;

public:
    LocalEnvironment(CoreEnvironment &core,
                     MainWindow &mw,
                     ThreadContext &context);

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

    ThreadContext &context() override {
        return _context;
    }

    Registers &registers() override {
        return _registers;
    }

    Project &project() override;

    CoreEnvironment &core() override;

    MainWindow &mainWindow() override {
        return _mainWindow;
    }

    const StandardCommands &standardCommands() const override;

    InteractionHandling &interactions() override;

    Editor &editor() override;

    //! Trigger redraw of winow
    void redrawScreen();

    void statusMessage(FStringView str) override;

    /// Shorthand function for context().guiQueue.addTask
    void guiTask(std::function<void()> f) override;
};
