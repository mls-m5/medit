#pragma once

#include <filesystem>
#include <functional>
#include <string_view>

class FifoFileListener {
public:
    using CallbackT = std::function<void(std::string)>;

    FifoFileListener(const FifoFileListener &) = delete;
    FifoFileListener(FifoFileListener &&) = delete;
    FifoFileListener &operator=(const FifoFileListener &) = delete;
    FifoFileListener &operator=(FifoFileListener &&) = delete;

    FifoFileListener(std::filesystem::path, CallbackT);
    ~FifoFileListener();

    void close();

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};
