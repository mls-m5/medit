#pragma once

#include <filesystem>
#include <fstream>
#include <functional>
#include <string_view>
#include <thread>

namespace lsp {

/// Handles communication with a open program through named pipes
struct Connection {
    using HandleFunctionT = std::function<void(std::istream &)>;
    using ExitedFunctionT = std::function<void()>;

    Connection(const Connection &) = delete;
    Connection(Connection &&) = delete;
    Connection &operator=(const Connection &) = delete;
    Connection &operator=(Connection &&) = delete;

    /// A new thread is created that gets a reference to the input stream
    /// ExitedFunction is called if the process was closed with a error return
    /// code
    Connection(std::string command, HandleFunctionT callback, ExitedFunctionT);

    /// Note that you need to
    ~Connection();

    /// @return true if error, false otherwise
    bool send(std::string_view);

    /// When it is not possible to request the child program to exit, just close
    /// the pipes
    void closePipes();

    operator bool() const;

private:
    void readIn(HandleFunctionT);
    void startProcess(std::string_view);

    std::filesystem::path _inPath;
    std::filesystem::path _outPath;
    std::filesystem::path _errorPath;

    std::ofstream _out;
    std::ifstream _in;
    std::ifstream _error;

    std::thread _thread;
    std::thread _clangdThread;
    std::thread _errorThread;

    bool _isServerRunning = false;

    ExitedFunctionT _exitedCallback;
};

} // namespace lsp
