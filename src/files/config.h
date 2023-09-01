#pragma once

#include <filesystem>
#include <optional>

/// Find or create a directory for medit files
std::optional<std::filesystem::path> localConfigDirectory(
    std::filesystem::path relativePath = {}, bool createIfNonexistent = false);

/// Look for files in some directory medit looks in
std::filesystem::path findConfig(std::filesystem::path file);

std::filesystem::path standardFifoDirectory();

inline const std::filesystem::path clientInPath =
    standardFifoDirectory() / "fifo-client-in";
inline const std::filesystem::path clientOutPath =
    standardFifoDirectory() / "fifo-client-out";
