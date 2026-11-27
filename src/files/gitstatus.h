#pragma once

#include <string>
#include <string_view>
#include <vector>

/// One entry of `git status --porcelain=v1 -z`. Paths are relative to the
/// repository root and exactly as on disk (no quoting).
struct GitStatusEntry {
    char index = ' ';    // X: staged state
    char worktree = ' '; // Y: unstaged state
    std::string path;
    std::string originalPath; // Source of a rename or copy, otherwise empty

    bool isDeleted() const {
        return index == 'D' || worktree == 'D';
    }

    bool isRenameOrCopy() const {
        return index == 'R' || index == 'C' || worktree == 'R' ||
               worktree == 'C';
    }

    bool operator==(const GitStatusEntry &) const = default;
};

/// Parses the output of `git status --porcelain=v1 -z`. Malformed trailing
/// data is ignored.
std::vector<GitStatusEntry> parseGitStatus(std::string_view output);
