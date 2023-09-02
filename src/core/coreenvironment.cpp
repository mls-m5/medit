#include "coreenvironment.h"
#include "core/fifofile.h"
#include "files/config.h"
#include "files/project.h"
#include "plugin/idebugger.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>

CoreEnvironment::CoreEnvironment(ThreadContext &context)
    : _context{&context}
    , _project{std::make_unique<Project>(files().directoryNotifications(),
                                         context.guiQueue())}
    , _consoleTtyPath{createFifo(standardConsoleTtyPipePath())}
    , _consoleInFile{_consoleTtyPath, [this](std::string data) {
                         if (_consoleCallback) {
                             _consoleCallback(std::move(data));
                         }
                     }} {
    cleanUpLocalPipes();

    std::ofstream{_consoleTtyPath.path()}
        << std::endl; // Prevent the listen file from waiting

    _project->updateCache(std::filesystem::current_path());
}

CoreEnvironment::~CoreEnvironment() {
    _consoleTtyPath.clear();
    _consoleInFile.close();
    std::ofstream{_consoleTtyPath.path()}
        << std::endl; // Prevent the listen file from waiting
}

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
