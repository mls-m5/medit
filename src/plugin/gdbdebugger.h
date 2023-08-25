#pragma once

#include "plugin/idebugger.h"
#include <functional>

class GdbDebugger : public IDebugger {
public:
    GdbDebugger();
    ~GdbDebugger() override;
    GdbDebugger(const GdbDebugger &) = delete;
    GdbDebugger(GdbDebugger &&) = delete;
    GdbDebugger &operator=(const GdbDebugger &) = delete;
    GdbDebugger &operator=(GdbDebugger &&) = delete;

    void run() override;
    void pause() override;
    void quit() override;

    void cont() override;
    void stepInto() override;
    void stepOver() override;
    void stepOut() override;

    void stateCallback(std::function<void(DebuggerState)>) override;
    void setBreakpoint(Path file, Position) override;
    void deleteBreakpoint(Path file, Position) override;

private:
    std::function<void(DebuggerState state)> _callback;
};
