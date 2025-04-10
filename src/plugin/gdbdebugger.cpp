#include "gdbdebugger.h"
#include "plugin/idebugger.h"
#include <filesystem>
#include <iostream>
#include <istream>
#include <mutex>
#include <regex>
#include <string>
#include <string_view>

GdbDebugger::GdbDebugger()
    : _connection{"gdb --interpreter=mi3",
                  [this](std::istream &in) { inputThread(in); },
                  [this]() { gdbExitedUnexpected(); }} {}

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

    updateBreakpointInfo();

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

void GdbDebugger::stateCallback(std::function<void(DebuggerState)> f) {
    _callback = f;
}

void GdbDebugger::breakpointListCallback(
    std::function<void(const BreakpointList)> f) {
    _breakpointListCallback = f;
}

void GdbDebugger::toggleBreakpoint(SourceLocation loc) {
    if (deleteBreakpoint(loc)) {
        return;
    }
    setBreakpoint(loc);
}

void GdbDebugger::updateBreakpointInfo() {
    // We cannot simply clear the list since we need to send a message to those
    // who need to remove breakpoints
    for (auto &it : _breakpointInfos) {
        it.second.clear();
    }
    _connection.send("info b"); // Request information about all set breakpoints
    // New breakpoint infos will be added in the input thread
    waitForDone();

    if (_breakpointListCallback) {
        _breakpointListCallback(_breakpointInfos);
    }
}

void GdbDebugger::setBreakpoint(SourceLocation loc) {
    _connection.send("b " + loc.path.string() + ":" +
                     std::to_string(loc.position.y() + 1));
    waitForDone();
    updateBreakpointInfo();
}

bool GdbDebugger::deleteBreakpoint(SourceLocation loc) {
    auto didFind = false;
    for (auto &file : _breakpointInfos) {
        auto ec = std::error_code{};
        if (!std::filesystem::equivalent(file.first, loc.path, ec)) {
            continue;
        }
        for (size_t i = 0; i < file.second.size();) {
            auto &info = file.second.at(i);
            if (loc.position.y() == info.lineNumber) {
                _connection.send("delete breakpoint " +
                                 std::to_string(info.breakpointNumber));
                waitForDone();
                file.second.erase(file.second.begin() + i);
                didFind = true;
            }
            else {
                ++i;
            }
        }
        break; // Only do one file
    }

    if (didFind) {
        updateBreakpointInfo();
    }
    return didFind;
}

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

void GdbDebugger::gdbExitedUnexpected() {
    if (_gdbStatusCallback) {
        _gdbStatusCallback("gdb exited");
    }
    _isRunning = false;
    // TODO: Make sure that this works
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

        if (line.starts_with("~")) {
            static const auto re = std::regex{
                R"(~"(\d+)\s+breakpoint\s+.* in (.+) at (.+):(\d+))"};

            auto match = std::smatch{};

            if (std::regex_search(line, match, re)) {
                auto file = match[3].str();
                _breakpointInfos[file].push_back({
                    .breakpointNumber = std::stoi(match[1].str()),
                    .functionSignature = match[2].str(),
                    .filePath = match[3].str(),
                    .lineNumber = std::stoul(match[4].str()) - 1,
                });
            }
        }

        if (line == "^running") {
            auto state = DebuggerState{};
            state.state = DebuggerState::Running;
            changeState(state);
        }

        // Example line:
        // Exception:
        // *stopped,reason="signal-received",signal-name="SIGABRT",signal-meaning="Aborted",frame={addr="0x00007ffff78969fc",func="__pthread_kill_implementation",args=[{name="no_tid",value="0"},{name="signo",value="6"},{name="threadid",value="140737352606656"}],file="./nptl/pthread_kill.c",fullname="./nptl/./nptl/pthread_kill.c",line="44",arch="i386:x86-64"},thread-id="1",stopped-threads="all",core="9"
        // After step:
        // *stopped,reason="breakpoint-hit",disp="keep",bkptno="1",frame={addr="0x000055555555534c",func="main",args=[{name="argc",value="1"},{name="argv",value="0x7fffffffe448"}],file="/home/mattias/Prog/Experiment/observable-pointer/src/main.cpp",fullname="/home/mattias/Prog/Experiment/observable-pointer/src/main.cpp",line="18",arch="i386:x86-64"},thread-id="1",stopped-threads="all",core="25"
        static const auto stopExpression = std::regex(
            R"_(stopped,reason="([^"]+)",.*?addr="([^"]+)",.*?func="([^"]+)",.*?file="([^"]+)",.*?fullname="([^"]+)",.*?line="([^"]+)")_");
        if (std::regex_search(line, matches, stopExpression)) {
            auto fullName = matches[5].str();
            auto line = matches[6].str();

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
