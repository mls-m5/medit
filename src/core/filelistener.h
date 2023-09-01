#pragma once

#include <filesystem>
#include <functional>
#include <string_view>

class FileListener {
    using CallbackT = std::function<void(std::string_view)>;

    FileListener(std::filesystem::path, CallbackT);

    void close();

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};
