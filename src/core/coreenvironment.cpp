#include "coreenvironment.h"
#include "core/fifofile.h"
#include "core/meditlog.h"
#include "files/config.h"
#include "files/project.h"
#include "plugin/idebugger.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <string_view>

CoreEnvironment::CoreEnvironment(ThreadContext &context)
    : _context{&context}
    , _project{std::make_unique<Project>(files().directoryNotifications(),
                                         context.guiQueue())}
    , _consoleTtyPath{createFifo(standardConsoleTtyPipePath())}
    , _consoleInFile{_consoleTtyPath,
                     [this](std::string data) { printToAllConsoles(data); }} {
    cleanUpLocalPipes();

    std::ofstream{_consoleTtyPath.path()}
        << std::endl; // Prevent the listen file from waiting

    _project->updateCache(std::filesystem::current_path());

    subscribeToLog(
        [this](std::string_view str) { printToAllConsoles(std::string{str}); });
}

CoreEnvironment::~CoreEnvironment() {
    _consoleCallback = {};
    unsubscribeToLog();
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

void CoreEnvironment::unsubscribeToConsoleCallback(void *ref) {
    auto it = std::remove_if(_consoleCallback.begin(),
                             _consoleCallback.end(),
                             [ref](auto &&a) { return a.second == ref; });

    _consoleCallback.erase(it);
}

void CoreEnvironment::printToAllConsoles(std::string text) {
    for (auto &cb : _consoleCallback) {
        if (cb.first) {
            cb.first(text);
        }
    }
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
