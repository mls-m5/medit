#pragma once

#include "lsp/connection.h"
#include "plugin/idebugger.h"
#include <condition_variable>
#include <filesystem>
#include <functional>
#include <iosfwd>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

class GdbDebugger : public IDebugger {
public:
    GdbDebugger();
    ~GdbDebugger() override;
    GdbDebugger(const GdbDebugger &) = delete;
    GdbDebugger(GdbDebugger &&) = delete;
    GdbDebugger &operator=(const GdbDebugger &) = delete;
    GdbDebugger &operator=(GdbDebugger &&) = delete;

    bool doesSupport(std::filesystem::path extension) override;

    void command(std::string_view command) override;

    /// Specify where to run the command
    void workingDirectory(std::filesystem::path) override;

    /// Application output
    void applicationOutputCallback(
        std::function<void(std::string_view)>) override;
    /// Only messages from the debugger
    void debuggerOutputCallback(std::function<void(std::string_view)>) override;

    void gdbStatusCallback(std::function<void(std::string_view)>) override;
    void stateCallback(std::function<void(DebuggerState)>) override;

    void run() override;
    void pause() override;
    void stop() override;

    void cont() override;
    void stepInto() override;
    void stepOver() override;
    void stepOut() override;

    void toggleBreakpoint(SourceLocation) override;
    void setBreakpoint(SourceLocation) override;
    void deleteBreakpoint(SourceLocation) override;

private:
    enum WaitResult {
        Done,
        Error,
    };

    WaitResult waitForDone();

    void changeState(DebuggerState state);

    std::function<void(DebuggerState state)> _callback;
    std::function<void(std::string_view)> _applicationOutputCallback;
    std::function<void(std::string_view)> _debuggerOutputCallback;
    std::function<void(std::string_view)> _gdbStatusCallback;

    void inputThread(std::istream &in);

    lsp::Connection _connection;

    std::string _debugCommand;
    std::string _debugArgs;
    std::filesystem::path _workingDirectory;

    enum RequestedInfo {
        None,
        BreakpointList,
    };

    RequestedInfo _requestedInfo = None;

    bool _isWaiting = false;
    std::mutex _waitMutex;
    std::condition_variable _waitVar;

    WaitResult _waitResult = Done;

    DebuggerState::State _currentState = DebuggerState::Stopped;

    bool _isRunning = true;
};
