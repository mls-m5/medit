#pragma once

#include <filesystem>
#include <set>

// One Git snapshot, obtained off the GUI thread. Paths are absolute and
// normalized so a tree row can look them up without invoking Git.
std::set<std::filesystem::path> changedGitFiles(
    const std::filesystem::path &root);
