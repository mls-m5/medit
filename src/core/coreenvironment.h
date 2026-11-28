#pragma once

#include "core/context.h"
#include "core/fifofilelistener.h"
#include "core/icommandrunner.h"
#include "core/logtype.h"
#include "core/plugins.h"
#include "files.h"
#include "files/ifilesystem.h"
#include "files/uniquefile.h"
#include "threadvalidation.h"
#include <filesystem>
#include <functional>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

/// Environment shared with all user of the program/server
class CoreEnvironment {
public:
    CoreEnvironment(ThreadContext &context,
                    std::filesystem::path projectToOpen);
    ~CoreEnvironment();

    CoreEnvironment(const CoreEnvironment &) = delete;
    CoreEnvironment(CoreEnvironment &&) = delete;
    CoreEnvironment &operator=(const CoreEnvironment &) = delete;
    CoreEnvironment &operator=(CoreEnvironment &&) = delete;

    ThreadContext &context();

    Plugins &plugins();

    Files &files();

    IDebugger *debugger();

    Project &project();

    /// Runs external programs without a shell; see ICommandRunner.
    ICommandRunner &commandRunner();

    /// All file access goes through this.
    IFileSystem &fileSystem() {
        return *_fileSystem;
    }

    void subscribeToLogCallback(
        std::function<void(LogType type, std::string)> f, void *ref);

    void unsubscribeToConsoleCallback(void *ref);

    void printToAllLogs(LogType, std::string text);

private:
    ThreadContext *_context =
        nullptr; // TODO: Handle lifetime of CoreEnvironment better
    // Before the plugins, which may keep a reference to it.
    std::unique_ptr<ICommandRunner> _commandRunner = createCommandRunner();
    IFileSystem *_fileSystem = &localFileSystem();
    Plugins _plugins;
    Files _files{*this};

    std::unique_ptr<Project> _project;

    ThreadValidation _tv{"core thread (gui thread)"};

    /// Move this out in the future and when there is more terminals that is
    /// handled this way
    UniqueFile _consoleTtyPath;
    UniqueFile _errorTtyPath;
    FifoFileListener _consoleInFile;
    FifoFileListener _errorInFile;
    std::vector<std::pair<std::function<void(LogType, std::string)>, void *>>
        _consoleCallback;
};
