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

#define STANDARD_COMMAND_LIST                                                  \
    STD_DEF(left);                                                             \
    STD_DEF(right);                                                            \
    STD_DEF(up);                                                               \
    STD_DEF(down);                                                             \
    STD_DEF(home);                                                             \
    STD_DEF(end);                                                              \
    STD_DEF(pageUp);                                                           \
    STD_DEF(pageDown);

#define STD_DEF(name) void (*name)(EnvPtrT env) = nullptr

/// A set of commands that can be bound to key presses and other events
struct StandardCommands {
    using EnvPtrT = std::shared_ptr<IEnvironment>;

    STANDARD_COMMAND_LIST

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
