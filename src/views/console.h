#pragma once

#include "meditfwd.h"
#include "text/buffer.h"
#include <functional>
#include <memory>
#include <string>

/// Just data to run commands in console (just a editor)
struct Console {
    FString openingMessage;
    std::string command;
    Buffer *buffer = nullptr;
    std::function<void()> callback;
    bool shouldShowConsole = true;
    bool shouldClear = true;

    /// Run a process and print the output to the console
    void run(std::shared_ptr<IEnvironment> env);
};
