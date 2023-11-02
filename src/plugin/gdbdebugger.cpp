#include "gdbdebugger.h"
#include "plugin/idebugger.h"
#include <filesystem>
#include <iostream>
#include <istream>
#include <mutex>
#include <ostream>
#include <regex>
#include <string>
#include <string_view>

GdbDebugger::GdbDebugger()
    : _connection{"gdb --interpreter=mi3",
                  [this](std::istream &in) { inputThread(in); }} {}

bool GdbDebugger::doesSupport(std::filesystem::path extension) {
    return extension == ".cpp" || extension == ".go";
}

void GdbDebugger::command(std::string_view c) {
    std::string command{c};
    _debugArgs = {};

    if (auto f = command.find(' '); f != std::string::npos) {
        _debugArgs = command.substr(f + 1);
        _debugCommand = command.substr(0, f);
    }
    else {
        _debugCommand = command;
    }
}

void GdbDebugger::workingDirectory(std::filesystem::path path) {
    _workingDirectory = path;
}

void GdbDebugger::applicationOutputCallback(
    std::function<void(std::string_view)> f) {
    _applicationOutputCallback = f;
}

void GdbDebugger::debuggerOutputCallback(
    std::function<void(std::string_view)> f) {
    _debuggerOutputCallback = f;
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

    if (_debugCommand.empty()) {
        if (_applicationOutputCallback) {
            _applicationOutputCallback(
                "no debug.command specified in .medit file");
            return;
        }
    }

    if (!_workingDirectory.empty()) {
        _connection.send(
            "cd " + std::filesystem::absolute(_workingDirectory).string() + "");

        if (waitForDone() == Error) {
            return;
        }
    }
    _connection.send("file " + _debugCommand);
    if (waitForDone() == Error) {
        return;
    }
    if (_debugArgs.empty()) {
        _connection.send("set args");
    }
    else {
        _connection.send("set args " + _debugArgs);
    }
    if (waitForDone() == Error) {
        return;
    }

    _connection.send("run &");
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

void GdbDebugger::toggleBreakpoint(SourceLocation loc) {
    // TODO: Handle unsetting
    setBreakpoint(loc);
}

void GdbDebugger::stateCallback(std::function<void(DebuggerState)> f) {
    _callback = f;
}

void GdbDebugger::setBreakpoint(SourceLocation loc) {
    _connection.send("b " + loc.path.string() + ":" +
                     std::to_string(loc.position.y() + 1));
    waitForDone();
    _connection.send("info b"); // Request information about all set breakpoints
    waitForDone();
}

void GdbDebugger::deleteBreakpoint(SourceLocation loc) {}

GdbDebugger::WaitResult GdbDebugger::waitForDone() {
    _isWaiting = true;
    auto lock = std::unique_lock{_waitMutex};
    _waitVar.wait(lock);
    return _waitResult;
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
                _waitResult = Done;
                _waitVar.notify_one();
            }
        }
        if (line.rfind("^error", 0) == 0) {
            if (_isWaiting) {
                _isWaiting = false;
                _waitResult = Error;
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

        if (_applicationOutputCallback) {
            // TODO: Separate applcation output from gdb output with gdb tty
            // command and special pipes

            bool isGdbOutput = false;

            if (!line.empty()) {
                auto first = line.front();
                switch (first) {
                case '^':
                case '*':
                case '+':
                case '=':
                case '~':
                case '@':
                case '&':
                    isGdbOutput = true;
                    break;
                default:
                    break;
                }
            }

            if (line.starts_with("(gdb)")) {
                isGdbOutput = true;
            }

            if (isGdbOutput) {
                _debuggerOutputCallback(line);
            }
            else {
                _applicationOutputCallback(line);
            }
        }

        // Implement the regex logic here:
    }
}
