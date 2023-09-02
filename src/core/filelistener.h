#pragma once

#include <filesystem>
#include <functional>
#include <string_view>

class FileListener {
public:
    using CallbackT = std::function<void(std::string)>;

    FileListener(const FileListener &) = delete;
    FileListener(FileListener &&) = delete;
    FileListener &operator=(const FileListener &) = delete;
    FileListener &operator=(FileListener &&) = delete;

    FileListener(std::filesystem::path, CallbackT);
    ~FileListener();

    void close();

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};
