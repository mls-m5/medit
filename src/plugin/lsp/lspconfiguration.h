#pragma once

#include <filesystem>
#include <functional>
#include <string>

struct LspConfiguration {
    LspConfiguration(const LspConfiguration &) = default;
    LspConfiguration(LspConfiguration &&) = default;
    LspConfiguration &operator=(const LspConfiguration &) = default;
    LspConfiguration &operator=(LspConfiguration &&) = default;
    LspConfiguration() = default;
    ~LspConfiguration() = default;

    /// Get the lsp configuration for the typical extension
    LspConfiguration(std::filesystem::path extension);

    std::string command;

    std::function<bool(std::filesystem::path)> isFileSupported;
};
