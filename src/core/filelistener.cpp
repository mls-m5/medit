#include "filelistener.h"
#include "core/threadname.h"
#include <filesystem>
#include <fstream>
#include <functional>
#include <string_view>
#include <thread>

struct FileListener::Impl {
    Impl(std::filesystem::path path, FileListener::CallbackT f)
        //        : f{f}
        : thread{std::thread{[this, path, f] { run(path, f); }}} {}

    void run(std::filesystem::path path, FileListener::CallbackT f) {
        setThreadName("tty file listener");
        while (isRunning) {
            auto file = std::ifstream{path};
            if (!file.is_open()) {
                return;
            }
            this->file = &file;
            if (!file) {
                throw std::runtime_error{"could not open file for listen"};
            }
            for (std::string line; std::getline(file, line);) {
                f(std::move(line));
            }
        }
        this->file = nullptr;
    }

    void close() {
        if (file && *file) {
            file->close();
        }
        thread.join();
    }

    bool isRunning = true;
    std::ifstream *file = nullptr;
    std::thread thread;
};

FileListener::FileListener(std::filesystem::path path, CallbackT callback)
    : _impl{std::make_unique<Impl>(path, callback)} {}

FileListener::~FileListener() {
    _impl->isRunning = false;
    close();
}

void FileListener::close() {
    _impl->close();
}
