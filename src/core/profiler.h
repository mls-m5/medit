#pragma once

#include <source_location>
#include <string_view>

/// Used in scope that you want to profile
struct ProfileDuration {
    /// Note that the profile scope function uses the pointer to
    /// the underlying data, and the user must therefore use constants that
    /// survives the whole execution the program, like with string literals
    /// The motivation for this is to save data and performance when used a lot
    ProfileDuration(
        std::string_view name = std::source_location::current().file_name(),
        std::string_view data2 =
            std::source_location::current().function_name(),
        int = std::source_location::current().line());

    ProfileDuration(const ProfileDuration &) = delete;
    ProfileDuration(ProfileDuration &&) = delete;
    ProfileDuration &operator=(const ProfileDuration &) = delete;
    ProfileDuration &operator=(ProfileDuration &&) = delete;

    ~ProfileDuration();

    std::string_view name;
    std::string_view data2;
    int line;
};

/// Here the value is stored, so you can use temporary data
void profileInstant(std::string_view value);

/// Just called once from main
void enableProfiling();
