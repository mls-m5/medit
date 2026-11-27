#pragma once

#include <atomic>
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
    // Invoked on the stderr reader thread; text does not include the newline.
    using ErrorFunctionT = std::function<void(std::string)>;

    Connection(const Connection &) = delete;
    Connection(Connection &&) = delete;
    Connection &operator=(const Connection &) = delete;
    Connection &operator=(Connection &&) = delete;

    /// A new thread is created that gets a reference to the input stream
    /// ExitedFunction is called if the process was closed with a error return
    /// code
    Connection(std::string command, HandleFunctionT callback, ExitedFunctionT,
               ErrorFunctionT error = {});

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

    std::thread _thread;
    std::thread _clangdThread;
    std::thread _errorThread;

    std::atomic<bool> _isServerRunning = false;

    ExitedFunctionT _exitedCallback;
};

} // namespace lsp
