#include "filelistener.h"
#include <fstream>
#include <functional>
#include <string_view>
#include <thread>

struct FileListener::Impl {
    Impl(std::filesystem::path path, FileListener::CallbackT f)
        : file{path}
        , thread{std::thread{[this] { run(); }}}
        , f{f} {}

    void run() {
        if (!file) {
            throw std::runtime_error{"could not open file for listen"};
        }
        for (std::string line; std::getline(file, line);) {
        }
    }

    void close() {
        if (file) {
            file.close();
        }
    }

    std::thread thread;
    std::ifstream file;
    FileListener::CallbackT f;
};

FileListener::FileListener(std::filesystem::path path, CallbackT callback)
    : _impl{std::make_unique<Impl>(path)} {}

void FileListener::close() {
    _impl->close();
}
