#include "lsp/connection.h"
#include "lsp/randomutil.h"
#include <filesystem>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
#include <utility>

using namespace std::literals;

namespace {
void createFifo(std::filesystem::path path) {
    auto command = "mkfifo "s + path.string();
    std::system(command.c_str());
}
} // namespace

namespace lsp {

Connection::Connection(std::string command,
                       HandleFunctionT handle,
                       ExitedFunctionT exitF,
                       ErrorFunctionT error)
    : _exitedCallback{exitF} {
    auto tmp = std::filesystem::temp_directory_path();
    _inPath = tmp / ("lsp-in-pipe-" + std::to_string(randomNumber(100000)));
    createFifo(_inPath);

    _outPath = tmp / ("lsp-out-pipe-" + std::to_string(randomNumber(100000)));
    createFifo(_outPath);

    _errorPath =
        tmp / ("lsp-error-pipe-" + std::to_string(randomNumber(100000)));
    createFifo(_errorPath);

    _clangdThread = std::thread{[this, command] { startProcess(command); }};
    _errorThread = std::thread{[this, error = std::move(error)] {
        std::ifstream stream{_errorPath};
        for (std::string line; std::getline(stream, line);) {
            if (error)
                error(std::move(line));
            else
                std::cerr << line << '\n';
        }
    }};

    _thread = std::thread{[handle, this] { readIn(handle); }};

    _out.open(_outPath);
}

Connection::~Connection() {
    // Wake a still-running protocol reader without blocking if it has already
    // reached EOF after a server crash.
    if (const int fd = ::open(_inPath.c_str(), O_WRONLY | O_NONBLOCK); fd >= 0) {
        (void)::write(fd, "\n", 1);
        ::close(fd);
    }

    _thread.join();
    _clangdThread.join();
    _errorThread.join();

    std::filesystem::remove(_inPath);
    std::filesystem::remove(_outPath);
    std::filesystem::remove(_errorPath);
}

bool Connection::send(std::string_view str) {
    _out << str << std::endl;
    if (_out.fail() || _out.bad()) {
        return true;
    }

    return false;
}

void Connection::closePipes() {
    _in.close();
    _out.close();
}

Connection::operator bool() const {
    return _isServerRunning;
}

void Connection::readIn(HandleFunctionT f) {
    _in.open(_inPath);

    f(_in);
}

void Connection::startProcess(std::string_view command) {
    auto ss = std::ostringstream{};
    ss << "( " << command << " ) > " << _inPath << " < " << _outPath << " 2> "
       << _errorPath;
    _isServerRunning = true;
    auto ret =
        std::system(ss.str().c_str()); // This is where the magic happends
    if (ret) {
        std::cerr << "failed to start " + std::string{command} + "\n";
        _exitedCallback();
    }
    _isServerRunning = false;
}

} // namespace lsp
