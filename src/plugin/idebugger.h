#pragma once

#include "text/position.h"
#include <filesystem>
#include <functional>
#include <string_view>

/// Specify a line in a file
struct SourceLocation {
    using Path = std::filesystem::path;

    Path path;

    /// Note: Medit positions starts at line 0 and column 0. conversion may be
    /// neded when interfacing with other programs
    Position position = {}; // Only used when paused

    operator bool() const {
        return path.empty();
    }
};

struct DebuggerState {

    enum State {
        Stopped,
        Running,
        Paused,
    };

    State state = Stopped;

    SourceLocation location;
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

    virtual bool doesSupport(std::filesystem::path) = 0;

    virtual void applicationOutputCallback(
        std::function<void(std::string_view)>) = 0;
    virtual void debuggerOutputCallback(
        std::function<void(std::string_view)>) = 0;
    virtual void stateCallback(std::function<void(DebuggerState)>) = 0;
    virtual void gdbStatusCallback(std::function<void(std::string_view)>) = 0;

    /// Specify where to run the command
    virtual void workingDirectory(std::filesystem::path) = 0;

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

    virtual void toggleBreakpoint(SourceLocation) = 0;
    virtual void setBreakpoint(SourceLocation) = 0;
    virtual void deleteBreakpoint(SourceLocation) = 0;

    // Watchpoints
    // Conditions

    // Print
    // Watch
};
