#pragma once

#include "lsp/connection.h"
#include "plugin/idebugger.h"
#include <condition_variable>
#include <functional>
#include <iosfwd>
#include <mutex>

class GdbDebugger : public IDebugger {
public:
    GdbDebugger();
    ~GdbDebugger() override;
    GdbDebugger(const GdbDebugger &) = delete;
    GdbDebugger(GdbDebugger &&) = delete;
    GdbDebugger &operator=(const GdbDebugger &) = delete;
    GdbDebugger &operator=(GdbDebugger &&) = delete;

    void command(std::string_view command) override;

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

    std::function<void(DebuggerState state)> _callback;

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
};
