#include "coreenvironment.h"
#include "plugin/idebugger.h"

CoreEnvironment *CoreEnvironment::_instance = nullptr;

CoreEnvironment::CoreEnvironment(ThreadContext &context)
    : _context{&context} {
    _instance = this;
}
CoreEnvironment::~CoreEnvironment() = default;

IDebugger *CoreEnvironment::debugger() {
    auto &debuggers = _plugins.get<IDebugger>();

    if (debuggers.empty()) {
        return nullptr;
    }

    // TODO: Make more generic for more debuggers in the futures
    return debuggers.front().get();
}

Files &CoreEnvironment::files() {
    return _files;
}

Plugins &CoreEnvironment::plugins() {
    return _plugins;
}

ThreadContext &CoreEnvironment::context() {
    return *_context;
}
