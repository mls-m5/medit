#include "coreenvironment.h"
#include "files/project.h"
#include "meditfwd.h"
#include "plugin/idebugger.h"
#include <memory>

CoreEnvironment::CoreEnvironment(ThreadContext &context)
    : _context{&context}
    , _project{std::make_unique<Project>(files().directoryNotifications())} {}
CoreEnvironment::~CoreEnvironment() = default;

IDebugger *CoreEnvironment::debugger() {
    auto &debuggers = _plugins.get<IDebugger>();

    if (debuggers.empty()) {
        return nullptr;
    }

    // TODO: Make more generic for more debuggers in the futures
    return debuggers.front().get();
}

Project &CoreEnvironment::project() {
    return *_project;
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
