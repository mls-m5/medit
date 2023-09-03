#include "fifofilelistener.h"
#include "core/threadname.h"
#include <filesystem>
#include <fstream>
#include <functional>
#include <ios>
#include <ostream>
#include <string_view>
#include <thread>

struct FifoFileListener::Impl {
    Impl(std::filesystem::path path, FifoFileListener::CallbackT f)
        : path{path}
        , thread{std::thread{[this, path, f] { run(path, f); }}} {}

    void run(std::filesystem::path path, FifoFileListener::CallbackT f) {
        setThreadName("tty file listener");
        while (isRunning) {
            file = std::ifstream{path};
            if (!file.is_open()) {
                return;
            }
            if (!std::filesystem::is_fifo(path)) {
                throw std::runtime_error{"file is not a fifo " + path.string()};
            }
            if (!file) {
                throw std::runtime_error{"could not open file for listen " +
                                         path.string()};
            }
            for (std::string line; std::getline(file, line);) {
                if (line.empty()) {
                    continue;
                }
                if (isRunning) {
                    f(std::move(line));
                }
            }
        }
    }

    void close() {
        if (!isRunning) {
            return;
        }
        isRunning = false;
        std::ofstream{path} << std::endl;
        thread.join();
    }

    std::ifstream file{};
    std::filesystem::path path;
    bool isRunning = true;
    std::thread thread;
};

FifoFileListener::FifoFileListener(std::filesystem::path path,
                                   CallbackT callback)
    : _impl{std::make_unique<Impl>(path, callback)} {}

FifoFileListener::~FifoFileListener() {
    close();
}

void FifoFileListener::close() {
    _impl->close();
}
