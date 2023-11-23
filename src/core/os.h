#pragma once

#include <filesystem>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>

enum Os {
    Linux,
    Windows,
    Emscripten,
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define MATMAKE_USING_WINDOWS
#endif
constexpr Os getOs() {
#ifdef MEDIT_USING_WINDOWS
    return Os::Windows;
#else
#ifdef __EMSCRIPTEN__
    return Os::Emscripten;
#else
    return Os::Linux;
#define MEDIT_USING_LINUX
#endif // __EMSCRIPTEN__
#endif
}

[[nodiscard]] inline bool hasCommand(std::string command) {
    if constexpr (getOs() == Os::Linux) {
        return !system(("command -v " + command + " > /dev/null").c_str());
    }
    else {
        throw std::runtime_error{std::string{__FILE__} + ":" +
                                 std::to_string(__LINE__) +
                                 " is not implemented "};
    }
}

[[nodiscard]] std::filesystem::path executablePath();

int runCommand(const std::string &command);
int runCommandAndCapture(const std::string &command);

int getPid();

bool isProcessRunning(int pid);

void setupSignals(std::function<void()> f);

/// Template version of run command that takes any number of arguments with
/// spaces between
template <typename... Args>
int runCommand(Args &&...args) {
    auto ss = std::ostringstream{};
    ((ss << args << " "), ...);
    auto str = ss.str();
    if (str.empty()) {
        return -1;
    }
    str.pop_back();
    return runCommand(static_cast<const std::string &>(str));
}

/// Run command and capture output to terminals
template <typename... Args>
int runCommandAndCapture(Args &&...args) {
    auto ss = std::ostringstream{};
    ((ss << args << " "), ...);
    auto str = ss.str();
    if (str.empty()) {
        return -1;
    }
    str.pop_back();
    return runCommandAndCapture(static_cast<const std::string &>(str));
}
