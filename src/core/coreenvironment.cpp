#include "coreenvironment.h"
#include "core/fifofile.h"
#include "files/config.h"
#include "files/project.h"
#include "plugin/idebugger.h"
#include <filesystem>
#include <fstream>
#include <memory>

CoreEnvironment::CoreEnvironment(ThreadContext &context)
    : _context{&context}
    , _project{std::make_unique<Project>(files().directoryNotifications(),
                                         context.guiQueue())}
    , _consoleTtyPath{createFifo(standardLocalFifoDirectory() /
                                 "console-tty-in")}
    , _consoleInFile{_consoleTtyPath, [](std::string_view data) {}} {

    _project->updateCache(std::filesystem::current_path());
}

CoreEnvironment::~CoreEnvironment() = default;

IDebugger *CoreEnvironment::debugger() {
    auto &debuggers = _plugins.get<IDebugger>();

    if (debuggers.empty()) {
        return nullptr;
    }

    auto lang = _project->projectExtension();

    for (auto &debugger : debuggers) {
        if (debugger->doesSupport(lang)) {
            return debugger.get();
        }
    }

    return {};
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
