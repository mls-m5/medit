#include "coreenvironment.h"
#include "plugin/idebugger.h"

CoreEnvironment *CoreEnvironment::_instance = nullptr;

CoreEnvironment::CoreEnvironment(ThreadContext &context)
    : _context{&context} {
    _instance = this;
}
CoreEnvironment::~CoreEnvironment() = default;

CoreEnvironment &CoreEnvironment::instance() {
    return *_instance;
}

IDebugger *CoreEnvironment::debugger() {
    auto &debuggers = _plugins.get<IDebugger>();

    if (debuggers.empty()) {
        return nullptr;
    }

    // TODO: Make more generic for more debuggers in the futures
    return debuggers.front().get();
}
