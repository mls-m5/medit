#pragma once

#include "text/position.h"
#include <filesystem>
#include <functional>
#include <string_view>

struct DebuggerState {
    using Path = std::filesystem::path;

    enum State {
        Stopped,
        Running,
        Paused,
    };

    State state = Stopped;
    Path path;
    Position position = {}; // Only used when paused
};

/// Abstraction for debuggers like gdb or pdb
class IDebugger {
public:
    using Path = std::filesystem::path;

    virtual ~IDebugger() = default;
    IDebugger(const IDebugger &) = delete;
    IDebugger(IDebugger &&) = delete;
    IDebugger &operator=(const IDebugger &) = delete;
    IDebugger &operator=(IDebugger &&) = delete;
    IDebugger() = default;

    virtual void applicationOutputCallback(
        std::function<void(std::string_view)>) = 0;
    virtual void stateCallback(std::function<void(DebuggerState)>) = 0;

    /// Set the command to be run
    virtual void command(std::string_view command) = 0;

    virtual void run() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    /// Note: continue is a c++ keyword
    virtual void cont() = 0;
    virtual void stepInto() = 0;
    virtual void stepOver() = 0;
    virtual void stepOut() = 0;

    virtual void toggleBreakpoint(Path file, Position) = 0;
    virtual void setBreakpoint(Path file, Position) = 0;
    virtual void deleteBreakpoint(Path file, Position) = 0;
    //    virtual void toggleBreakpoint(File &file, Position);

    // Watchpoints
    // Conditions

    // Print
    // Watch
};
