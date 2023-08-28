#pragma once

#include "lsp/connection.h"
#include "plugin/idebugger.h"
#include <condition_variable>
#include <functional>
#include <iosfwd>
#include <mutex>
#include <string_view>

class GdbDebugger : public IDebugger {
public:
    GdbDebugger();
    ~GdbDebugger() override;
    GdbDebugger(const GdbDebugger &) = delete;
    GdbDebugger(GdbDebugger &&) = delete;
    GdbDebugger &operator=(const GdbDebugger &) = delete;
    GdbDebugger &operator=(GdbDebugger &&) = delete;

    void command(std::string_view command) override;

    /// Application output
    void applicationOutputCallback(
        std::function<void(std::string_view)>) override;

    void gdbStatusCallback(std::function<void(std::string_view)>) override;

    void run() override;
    void pause() override;
    void stop() override;

    void cont() override;
    void stepInto() override;
    void stepOver() override;
    void stepOut() override;

    void toggleBreakpoint(Path file, Position) override;
    void stateCallback(std::function<void(DebuggerState)>) override;
    void setBreakpoint(Path file, Position) override;
    void deleteBreakpoint(Path file, Position) override;

private:
    void waitForDone();

    void changeState(DebuggerState state);

    std::function<void(DebuggerState state)> _callback;
    std::function<void(std::string_view)> _applicationOutputCallback;
    std::function<void(std::string_view)> _gdbStatusCallback;

    void inputThread(std::istream &in);

    lsp::Connection _connection;

    std::string _debugCommand;

    enum RequestedInfo {
        None,
        BreakpointList,
    };

    RequestedInfo _requestedInfo = None;

    bool _isWaiting = false;
    std::mutex _waitMutex;
    std::condition_variable _waitVar;

    DebuggerState::State _currentState = DebuggerState::Stopped;

    bool _isRunning = true;
};
