#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

struct LspConfiguration {
    LspConfiguration(const LspConfiguration &) = default;
    LspConfiguration(LspConfiguration &&) = default;
    LspConfiguration &operator=(const LspConfiguration &) = default;
    LspConfiguration &operator=(LspConfiguration &&) = default;
    //    LspConfiguration() = default;
    ~LspConfiguration() = default;

    // This is the entry point
    static std::optional<LspConfiguration> getConfiguration(
        std::filesystem::path path);

    std::string command;

    std::function<bool(std::filesystem::path)> isFileSupported;

private:
    /// Get the lsp configuration for the typical extension
    //    LspConfiguration(std::filesystem::path extension);
    LspConfiguration() = default;
};
