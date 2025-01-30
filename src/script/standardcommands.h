#pragma once

#include "ienvironment.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

// This list should contain only core commands, the rest is registered through
// StaticCommandRegister or some other method
#define STANDARD_COMMAND_LIST                                                  \
    STD_DEF(left)                                                              \
    STD_DEF(right)                                                             \
    STD_DEF(up)                                                                \
    STD_DEF(down)                                                              \
    STD_DEF(home)                                                              \
    STD_DEF(end)                                                               \
    STD_DEF(page_up)                                                           \
    STD_DEF(page_down)                                                         \
    STD_DEF(switch_header)                                                     \
    STD_DEF(yank_line)                                                         \
    STD_DEF(delete_line)                                                       \
    STD_DEF(delete_word)                                                       \
    STD_DEF(change_word)                                                       \
    STD_DEF(clear_line)                                                        \
    STD_DEF(insert)                                                            \
    STD_DEF(insert_at_beginning)                                               \
    STD_DEF(split)                                                             \
    STD_DEF(yank)                                                              \
    STD_DEF(yank_block)                                                        \
    STD_DEF(backspace)                                                         \
    STD_DEF(erase)                                                             \
    STD_DEF(erase_after)                                                       \
    STD_DEF(erase_before_word)                                                 \
    STD_DEF(delete_until_end_of_line)                                          \
    STD_DEF(change_until_end_of_line)                                          \
    STD_DEF(paste_before)                                                      \
    STD_DEF(paste)                                                             \
    STD_DEF(join)                                                              \
    STD_DEF(convert_to_upper)                                                  \
    STD_DEF(convert_to_lower)                                                  \
    STD_DEF(toggle_case)                                                       \
    STD_DEF(format)                                                            \
    STD_DEF(save)                                                              \
    STD_DEF(copy_indentation)                                                  \
    STD_DEF(close_brace)                                                       \
    STD_DEF(undo)                                                              \
    STD_DEF(undo_major)                                                        \
    STD_DEF(redo)                                                              \
    STD_DEF(build)                                                             \
    STD_DEF(run)                                                               \
    STD_DEF(quit)                                                              \
    STD_DEF(reveal_file_in_explorer)                                           \
    STD_DEF(insert_mode)                                                       \
    STD_DEF(normal_mode)                                                       \
    STD_DEF(visual_mode)                                                       \
    STD_DEF(visual_block_mode)                                                 \
    STD_DEF(toggle_visual_end)                                                 \
    STD_DEF(toggle_comment)                                                    \
    STD_DEF(select_word)                                                       \
    STD_DEF(select_inner_word)                                                 \
    STD_DEF(select_inner_paren)                                                \
    STD_DEF(select_around_paren)                                               \
    STD_DEF(select_all)                                                        \
    STD_DEF(new_file)                                                          \
    STD_DEF(indent)                                                            \
    STD_DEF(deindent)                                                          \
    STD_DEF(back)                                                              \
    STD_DEF(forward)                                                           \
    STD_DEF(debug_run)                                                         \
    STD_DEF(debug_pause)                                                       \
    STD_DEF(debug_stop)                                                        \
    STD_DEF(debug_continue)                                                    \
    STD_DEF(debug_step_into)                                                   \
    STD_DEF(debug_step_over)                                                   \
    STD_DEF(debug_step_out)                                                    \
    STD_DEF(debug_toggle_breakpoint)

#define STD_DEF(name) std::function<void(EnvPtrT env)> name;

/// A set of commands that can be bound to key presses and other events
struct StandardCommands {
    using EnvPtrT = std::shared_ptr<IEnvironment>;

    /// Define standard commands
    STANDARD_COMMAND_LIST

    /// Open a file in the current active text editor
    void (*open)(EnvPtrT env,
                 std::filesystem::path,
                 std::optional<int> x,
                 std::optional<int> y) = nullptr;

    struct FunctionStruct {
        std::function<void(EnvPtrT)> f;
        void *ref = nullptr;

        void operator=(const decltype(f) &func) {
            f = func;
        }
    };

    std::unordered_map<std::string, FunctionStruct> namedCommands;

    /// Get a instance of the StandardCommands struct
    /// This function is only available for the main lib,
    /// A reference can be obtained from IEnvironment for plugins
    static StandardCommands &get();

    // TODO: Handle commands not loaded in some better way
    static inline const std::function<void(EnvPtrT)> doNothing = [](EnvPtrT) {};

    const std::function<void(EnvPtrT)> &f(const std::string &name);

    /// Combine several standard commands into one single function
    template <typename... Args>
    static std::function<void(EnvPtrT)> combine(Args... args) {
        return [args...](EnvPtrT e) { ((args(e)), ...); };
    }

    void addCommand(std::string_view name,
                    std::function<void(EnvPtrT)> f,
                    void *ref);

    void removeCommand(void *ref);
};

#undef STD_DEF
