#pragma once

#include "core/context.h"
#include "core/plugins.h"
#include "files.h"
#include "threadvalidation.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

/// Environment shared with all user of the program/server
class CoreEnvironment {
public:
    CoreEnvironment();

    CoreEnvironment(const CoreEnvironment &) = delete;
    CoreEnvironment(CoreEnvironment &&) = delete;
    CoreEnvironment &operator=(const CoreEnvironment &) = delete;
    CoreEnvironment &operator=(CoreEnvironment &&) = delete;

    static CoreEnvironment &instance();

    ThreadContext &context() {
        return *_context;
    }

    void context(ThreadContext *c) {
        _context = c;
    }

    Plugins &plugins() {
        return _plugins;
    }

    Files &files() {
        return _files;
    }

private:
    Plugins _plugins;
    Files _files{*this};

    ThreadValidation _tv{"core thread (gui thread)"};

    ThreadContext *_context =
        nullptr; // TODO: Handle lifetime of CoreEnvironment better
};
