#pragma once

#include "ienvironment.h"
#include "iscope.h"
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

void addStandardCommands(IScope &env);

struct StandardCommands {
    /// Open a file in the current active text editor
    void (*open)(IEnvironment &env,
                 std::filesystem::path,
                 std::optional<int> x,
                 std::optional<int> y);

    /// Get a instance of the StandardCommands struct
    /// This function is only available for the main lib,
    /// A reference can be obtained from IEnvironment for plugins
    static StandardCommands &get();
};
