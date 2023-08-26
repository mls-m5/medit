#include "gdbdebugger.h"
#include <iostream>
#include <istream>
#include <ostream>
#include <string>

GdbDebugger::GdbDebugger()
    : _connection{"gdb --interpreter=mi3",
                  [this](std::istream &in) { inputThread(in); }} {

    // TODO this is for testing only. Remove this
    GdbDebugger::command("./test/medit_tests Utf8Char");
    //    GdbDebugger::run();
}

void GdbDebugger::command(std::string_view c) {
    std::string command{c};
    auto args = std::string{};

    if (auto f = command.find(' '); f != std::string::npos) {
        args = command.substr(f + 1);
        command = command.substr(0, f);
    }
    _connection.send("file " + command + "\n");
    _connection.send("set args " + args);
}

GdbDebugger::~GdbDebugger() {
    _connection.send("quit\n");
}

void GdbDebugger::run() {
    _connection.send("run " + _debugCommand + "\n");
}

void GdbDebugger::pause() {
    _connection.send("pause\n");
}

void GdbDebugger::stop() {
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
