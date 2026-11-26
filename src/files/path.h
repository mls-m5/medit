#pragma once

#include <filesystem>

using Path = std::filesystem::path;

// Stable lexical identity, without filesystem traversal or symlink resolution.
// Strip trailing separators except at the filesystem root; empty stays empty.
inline Path normalizedAbsolutePath(const Path &path) {
    if (path.empty())
        return {};
    auto result = std::filesystem::absolute(path).lexically_normal();
    if (result != result.root_path() && result.filename().empty())
        result = result.parent_path();
    return result;
}
