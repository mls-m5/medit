#include "gdbdebugger.h"
#include <iostream>
#include <istream>
#include <ostream>
#include <string>

GdbDebugger::GdbDebugger()
    : _connection{"gdb --interpreter=mi3",
                  [this](std::istream &in) { inputThread(in); }} {
    run();
    quit();
}

GdbDebugger::~GdbDebugger() = default;

void GdbDebugger::run() {
    _connection.send("run\n");
}

void GdbDebugger::pause() {
    _connection.send("pouse\n");
}

void GdbDebugger::quit() {
    _connection.send("quit\n");
}

void GdbDebugger::cont() {
    _connection.send("c\n");
}

void GdbDebugger::stepInto() {}

void GdbDebugger::stepOver() {
    _connection.send("n");
}

void GdbDebugger::stepOut() {}

void GdbDebugger::stateCallback(std::function<void(DebuggerState)> f) {
    _callback = f;
}

void GdbDebugger::setBreakpoint(Path file, Position) {}

void GdbDebugger::deleteBreakpoint(Path file, Position) {}

void GdbDebugger::inputThread(std::istream &in) {
    for (std::string line; std::getline(in, line);) {
        std::cout << line << std::endl;
    }
}
