#include "gdbdebugger.h"

GdbDebugger::GdbDebugger() = default;

GdbDebugger::~GdbDebugger() = default;

void GdbDebugger::run() {}

void GdbDebugger::pause() {}

void GdbDebugger::quit() {}

void GdbDebugger::cont() {}

void GdbDebugger::stepInto() {}

void GdbDebugger::stepOver() {}

void GdbDebugger::stepOut() {}

void GdbDebugger::stateCallback(std::function<void(DebuggerState)> f) {
    _callback = f;
}

void GdbDebugger::setBreakpoint(Path file, Position) {}

void GdbDebugger::deleteBreakpoint(Path file, Position) {}
