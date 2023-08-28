#include "gdbdebugger.h"
#include "plugin/idebugger.h"
#include <iostream>
#include <istream>
#include <mutex>
#include <ostream>
#include <regex>
#include <string>
#include <string_view>

GdbDebugger::GdbDebugger()
    : _connection{"gdb --interpreter=mi3",
                  [this](std::istream &in) { inputThread(in); }} {

    GdbDebugger::command("./test/medit_tests Utf8Char");
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

void GdbDebugger::applicationOutputCallback(
    std::function<void(std::string_view)> f) {
    _applicationOutputCallback = f;
}

void GdbDebugger::gdbStatusCallback(std::function<void(std::string_view)> f) {
    _gdbStatusCallback = f;
}

GdbDebugger::~GdbDebugger() {
    _isRunning = false;
    _applicationOutputCallback = {};
    _callback = {};
    _gdbStatusCallback = {};
    _connection.send("quit\n");
}

void GdbDebugger::run() {
    if (_currentState == DebuggerState::Paused) {
        cont();
        return;
    }
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
    waitForDone();
}

void GdbDebugger::deleteBreakpoint(Path file, Position) {}

void GdbDebugger::waitForDone() {
    _isWaiting = true;
    auto lock = std::unique_lock{_waitMutex};
    _waitVar.wait(lock);
}

void GdbDebugger::changeState(DebuggerState state) {
    _currentState = state.state;
    if (_callback) {
        _callback(state);
    }
}

void GdbDebugger::inputThread(std::istream &in) {

    std::smatch matches;

    for (std::string line; std::getline(in, line);) {
        if (!_isRunning) {
            return;
        }
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

        if (line.rfind("=", 0) == 0) {
            if (_gdbStatusCallback) {
                _gdbStatusCallback(line.substr(1, line.find(",")));
            }
        }

        constexpr auto breakpointStr = std::string_view{"~\"Breakpoint "};
        if (line.rfind(breakpointStr, 0) == 0) {
            static const std::regex breakpointRegex(
                R"~(~"Breakpoint (\d+) at (0x[0-9a-fA-F]+): file (.*), line (\d+).\\n")~");
            if (std::regex_match(line, matches, breakpointRegex)) {
                //                std::cout << "Breakpoint Number: " <<
                //                matches[1].str()
                //                          << std::endl;
                //                std::cout << "Address: " << matches[2].str()
                //                << std::endl; std::cout << "Filename: " <<
                //                matches[3].str() << std::endl; std::cout <<
                //                "Line: " << matches[4].str() << std::endl;
            }
        }

        if (line == "^running") {
            auto state = DebuggerState{};
            state.state = DebuggerState::Running;
            changeState(state);
        }

        static const auto stopExpression = std::regex(
            R"_(stopped,reason="([^"]+)",.*?addr="([^"]+)",.*?func="([^"]+)",.*?file="([^"]+)",.*?fullname="([^"]+)",.*?line="([^"]+)")_");
        if (std::regex_search(line, matches, stopExpression)) {
            //            std::cout << "Reason: " << matches[1].str() <<
            //            std::endl; std::cout << "Address: " <<
            //            matches[2].str() << std::endl; std::cout << "Function:
            //            " << matches[3].str() << std::endl; std::cout <<
            //            "File: " << matches[4].str() << std::endl;
            auto fullName = matches[5].str();
            //            std::cout << "Fullname: " << fullName << std::endl;
            auto line = matches[6].str();
            //            std::cout << "Line: " << line << std::endl;

            auto state = DebuggerState{};
            state.location.position = {0, std::stoul(line) - 1};
            state.location.path = fullName;

            state.state = DebuggerState::Paused;
            changeState(state);
        }
        else if (line.rfind("*stopped", 0) == 0) {
            changeState({});
        }

        //        std::cout << line << std::endl;

        if (_applicationOutputCallback) {
            // TODO: Separate applcation output from gdb output with gdb tty
            // command and special pipes
            _applicationOutputCallback(line);
        }

        // Implement the regex logic here:
    }
}
