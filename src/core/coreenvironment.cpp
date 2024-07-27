#include "coreenvironment.h"
#include "core/fifofile.h"
#include "core/logtype.h"
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

CoreEnvironment::CoreEnvironment(ThreadContext &context,
                                 std::filesystem::path projectPath)
    : _context{&context}
    , _project{std::make_unique<Project>(
          projectPath, files().directoryNotifications(), context.guiQueue())}
    , _consoleTtyPath{createFifo(standardConsoleTtyPipePath())}
    , _errorTtyPath{createFifo(standardErrorTtyPipePath())}
    , _consoleInFile{_consoleTtyPath,
                     [this](std::string data) {
                         printToAllLogs(LogType::Info, data);
                     }}
    , _errorInFile{_errorTtyPath, [this](std::string data) {
                       printToAllLogs(LogType::Error, data);
                   }} {
    cleanUpOldLocalPipes();

    std::ofstream{_consoleTtyPath.path()}
        << std::endl; // Prevent the listen file from waiting

    _project->updateCache();

    subscribeToLog([this](LogType type, std::string_view str) {
        printToAllLogs(type, std::string{str});
    });
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

void CoreEnvironment::subscribeToLogCallback(
    std::function<void(LogType, std::string)> f, void *ref) {
    _consoleCallback.push_back({f, ref});
}

void CoreEnvironment::unsubscribeToConsoleCallback(void *ref) {
    auto it = std::remove_if(_consoleCallback.begin(),
                             _consoleCallback.end(),
                             [ref](auto &&a) { return a.second == ref; });

    _consoleCallback.erase(it, _consoleCallback.end());
}

void CoreEnvironment::printToAllLogs(LogType type, std::string text) {
    for (auto &cb : _consoleCallback) {
        if (cb.first) {
            cb.first(type, text);
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
