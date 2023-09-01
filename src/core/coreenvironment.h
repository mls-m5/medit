#pragma once

#include "core/context.h"
#include "core/plugins.h"
#include "files.h"
#include "files/uniquefile.h"
#include "threadvalidation.h"
#include <filesystem>
#include <functional>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <vector>

/// Environment shared with all user of the program/server
class CoreEnvironment {
public:
    CoreEnvironment(ThreadContext &context);
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

private:
    ThreadContext *_context =
        nullptr; // TODO: Handle lifetime of CoreEnvironment better
    Plugins _plugins;
    Files _files{*this};

    std::unique_ptr<Project> _project;

    ThreadValidation _tv{"core thread (gui thread)"};

    UniqueFile _consoleTty;
    std::unique_ptr<std::ifstream> _consoleInFile;
};
