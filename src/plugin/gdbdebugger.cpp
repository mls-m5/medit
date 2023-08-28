#include "gdbdebugger.h"
#include <iostream>
#include <istream>
#include <mutex>
#include <ostream>
#include <string>
#include <string_view>

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
    _connection.send("run " + _debugCommand + "&");
}

void GdbDebugger::pause() {
    _connection.send("interrupt");
}

void GdbDebugger::stop() {
    _connection.send("quit");
    _connection.closePipes();
}

void GdbDebugger::cont() {
    _connection.send("c &");
}

void GdbDebugger::stepInto() {
    _connection.send("step &");
}

void GdbDebugger::stepOver() {
    _connection.send("n &");
}

void GdbDebugger::stepOut() {
    _connection.send("finish &");
}

void GdbDebugger::toggleBreakpoint(Path file, Position pos) {
    // TODO: Handle unsetting
    setBreakpoint(file, pos);
}

void GdbDebugger::stateCallback(std::function<void(DebuggerState)> f) {
    _callback = f;
}

void GdbDebugger::setBreakpoint(Path file, Position pos) {
    _connection.send("b " + file.string() + ":" + std::to_string(pos.y() + 1));
    waitForDone();
    _connection.send("info b"); // Request information about all set breakpoints
}

void GdbDebugger::deleteBreakpoint(Path file, Position) {}

void GdbDebugger::waitForDone() {
    _isWaiting = true;
    auto lock = std::unique_lock{_waitMutex};
    _waitVar.wait(lock);
}

void GdbDebugger::inputThread(std::istream &in) {
    for (std::string line; std::getline(in, line);) {
        if (line.empty()) {
            continue;
        }
        if (line.back() == '\r') {
            line.pop_back();
        }
        if (line == "^done") {
            if (_isWaiting) {
                _isWaiting = false;
                _waitVar.notify_one();
            }
        }

        constexpr auto breakpointStr = std::string_view{"~\"Breakpoint "};
        if (line.rfind(breakpointStr, 0) == 0) {
            line = line.substr(breakpointStr.size());
        }
        std::cout << line << std::endl;
    }
}
