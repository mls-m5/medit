#pragma once

#include "ienvironment.h"
#include "iscope.h"
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

void addStandardCommands(IScope &env);

#define STD_DEF(name) void (*name)(EnvPtrT env) = nullptr

/// A set of commands that can be bound to key presses and other events
struct StandardCommands {
    using EnvPtrT = std::shared_ptr<IEnvironment>;

    //    void (*left)(EnvPtrT env);
    STD_DEF(left);

    /// Open a file in the current active text editor
    void (*open)(EnvPtrT env,
                 std::filesystem::path,
                 std::optional<int> x,
                 std::optional<int> y) = nullptr;

    //    void (*selectInnerWord)(EnvPtrT env);
    STD_DEF(selectInnerWord);

    /// Get a instance of the StandardCommands struct
    /// This function is only available for the main lib,
    /// A reference can be obtained from IEnvironment for plugins
    static StandardCommands &get();
};

#undef STD_DEF
