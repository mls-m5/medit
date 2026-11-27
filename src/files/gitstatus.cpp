#include "files/gitstatus.h"

std::vector<GitStatusEntry> parseGitStatus(std::string_view output) {
    auto entries = std::vector<GitStatusEntry>{};

    // Each entry is "XY PATH\0", followed by "ORIG_PATH\0" for renames and
    // copies.
    auto next = [&output]() -> std::string_view {
        const auto end = output.find('\0');
        if (end == std::string_view::npos) {
            const auto rest = output;
            output = {};
            return rest;
        }
        const auto field = output.substr(0, end);
        output.remove_prefix(end + 1);
        return field;
    };

    while (!output.empty()) {
        const auto field = next();
        if (field.size() < 4 || field[2] != ' ') {
            continue;
        }
        auto entry = GitStatusEntry{
            .index = field[0],
            .worktree = field[1],
            .path = std::string{field.substr(3)},
            .originalPath = {},
        };
        if (entry.isRenameOrCopy()) {
            entry.originalPath = std::string{next()};
        }
        entries.push_back(std::move(entry));
    }
    return entries;
}
