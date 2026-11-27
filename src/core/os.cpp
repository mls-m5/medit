
#include "os.h"
#include "files/config.h"
#include <array>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <ostream>

namespace {

std::function<void()> signalCallback;

}

#ifdef MEDIT_USING_WINDOWS

#include <windows.h>

filesystem::Path getPath() {
    return {}; // Not implemented
}

int getPid() {
    GetCurrentProcessId();
}

void setupSignals(std::function<void()> f) {
    static_static_assert(false, "not implemented");
}

void restartWithArguments(std::vector<std::string> arguments) {
    std::cout << "restart is not implemented in windows yet";
}

void revealInExplorer(std::filesystem::path path) {
    std::cout << "showing folder is not implemented in windows yet";
}

#else

#include <filesystem>
#include <signal.h>
#include <unistd.h>

bool hasCommand(std::string_view command) {
    if (command.empty()) {
        return false;
    }
    const auto isExecutable = [](const std::filesystem::path &path) {
        auto error = std::error_code{};
        return std::filesystem::is_regular_file(path, error) &&
               ::access(path.c_str(), X_OK) == 0;
    };
    if (command.find('/') != std::string_view::npos) {
        return isExecutable(std::filesystem::path{command});
    }
    const auto *pathVariable = std::getenv("PATH");
    if (!pathVariable) {
        return false;
    }
    auto directories = std::string_view{pathVariable};
    for (;;) {
        const auto separator = directories.find(':');
        const auto directory = directories.substr(0, separator);
        // An empty entry means the current directory.
        const auto base = directory.empty() ? std::filesystem::path{"."}
                                            : std::filesystem::path{directory};
        if (isExecutable(base / command)) {
            return true;
        }
        if (separator == std::string_view::npos) {
            return false;
        }
        directories.remove_prefix(separator + 1);
    }
}

std::filesystem::path getPath() {
    auto possibleLinks = std::array<std::filesystem::path, 3>{
        "/proc/self/exe",
        "/proc/curproc/file",
        "/proc/self/path/a.out",
    };

    for (auto &link : possibleLinks) {
        if (std::filesystem::exists(link) ||
            std::filesystem::is_symlink(link)) {
            return std::filesystem::read_symlink(link);
        }
    }

    return {};
}

int getPid() {
    return getpid();
}

bool isProcessRunning(int pid) {
    auto path = std::filesystem::path{"/proc/" + std::to_string(pid)};
    return std::filesystem::exists(path);
}

void sigintHandler(int signum) {
    std::puts("received interrupt");
    signalCallback();
}

void setupSignals(std::function<void()> f) {
    signal(SIGPIPE, SIG_IGN);
    signalCallback = f;
    signal(SIGINT, sigintHandler);
}

#endif

std::filesystem::path executablePath() {
    return getPath();
}

int runCommand(const std::string &command) {
    return std::system(command.c_str());
}

int runCommandAndCapture(const std::string &command) {
    auto newCommand =
        (command + " >\"" + standardConsoleTtyPipePath().string() + "\" 2>\"" +
         standardErrorTtyPipePath().string() + "\"");
    return std::system(newCommand.c_str());
}

void restartWithArguments(std::vector<std::string> arguments) {
    auto program = executablePath();

    // Convert std::vector<std::string> to char* array
    auto argv = std::vector<char *>{};
    argv.push_back(const_cast<char *>(program.c_str()));
    for (const auto &arg : arguments) {
        argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr); // null-terminated array

    // Execute the new program
    execv(program.c_str(), argv.data());

    // If execv returns, it must have failed
    std::cerr << "failed to restart\n" << std::endl;
    std::exit(EXIT_FAILURE);
}

void revealInExplorer(std::filesystem::path path) {
    if (path.empty()) {
        return;
    }

    std::filesystem::path directory =
        path.has_parent_path() ? path.parent_path() : path;

    // List of commands to try
    std::vector<std::string> commands = {
        "thunar \"" + path.string() + "\"",
        "nautilus --select \"" + path.string() + "\"",
        "dolphin --select \"" + path.string() + "\"",
        "xdg-open \"" + directory.string() + "\"",
    };

    for (const auto &command : commands) {
        if (std::system(command.c_str()) == 0) {
            return;
        }
    }

    std::cerr << "Failed to reveal file: " << path << std::endl;
}
