#pragma once

#include "ienvironment.h"
#include "iscope.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
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
    STD_DEF(pageDown);                                                         \
    STD_DEF(wordBegin);                                                        \
    STD_DEF(wordEnd);                                                          \
    STD_DEF(switchHeader);                                                     \
    STD_DEF(selectInnerWord);                                                  \
    STD_DEF(yankLine);                                                         \
    STD_DEF(deleteLine);                                                       \
    STD_DEF(clearLine);                                                        \
    STD_DEF(insert);                                                           \
    STD_DEF(split);                                                            \
    STD_DEF(yank);                                                             \
    STD_DEF(erase);                                                            \
    STD_DEF(eraseBeforeWord);                                                  \
    STD_DEF(pasteBefore);

#define STD_DEF(name) std::function<void(EnvPtrT env)> name

/// A set of commands that can be bound to key presses and other events
struct StandardCommands {
    using EnvPtrT = std::shared_ptr<IEnvironment>;

    STANDARD_COMMAND_LIST

    /// Open a file in the current active text editor
    void (*open)(EnvPtrT env,
                 std::filesystem::path,
                 std::optional<int> x,
                 std::optional<int> y) = nullptr;

    std::unordered_map<std::string, std::function<void(EnvPtrT env)>>
        namedCommands;

    /// Get a instance of the StandardCommands struct
    /// This function is only available for the main lib,
    /// A reference can be obtained from IEnvironment for plugins
    static StandardCommands &get();
};

#undef STD_DEF
