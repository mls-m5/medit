#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

/// Line-based diff and three-way merge, used to update a buffer from a file
/// that changed on disk without touching the user's unsaved edits.
///
/// Texts are split on '\n' only, so "a\nb" and "a\nb\n" differ in their last
/// (empty) line, and '\r' stays part of a line.

/// The lines of `text`; always at least one (possibly empty) line.
std::vector<std::string_view> splitLines(std::string_view text);

/// Lines [fromBegin, fromEnd) of the old text are replaced by lines
/// [toBegin, toEnd) of the new text. Either range may be empty.
struct DiffHunk {
    size_t fromBegin = 0;
    size_t fromEnd = 0;
    size_t toBegin = 0;
    size_t toEnd = 0;

    bool operator==(const DiffHunk &) const = default;
};

/// The hunks that turn `from` into `to`, in order. Uses Myers' algorithm after
/// trimming the common start and end; above `maxEdits` differing lines it
/// gives up on a minimal diff and returns the whole differing middle as one
/// hunk.
std::vector<DiffHunk> diffLines(const std::vector<std::string_view> &from,
                                const std::vector<std::string_view> &to,
                                size_t maxEdits = 4000);

/// Replace lines [begin, end) with `lines`.
struct LineEdit {
    size_t begin = 0;
    size_t end = 0;
    std::vector<std::string> lines;

    bool operator==(const LineEdit &) const = default;
};

/// A range of lines [begin, end); an empty range marks the position between
/// two lines.
struct LineRange {
    size_t begin = 0;
    size_t end = 0;

    bool operator==(const LineRange &) const = default;
};

struct LineMerge {
    /// True when the changes overlap; then `edits` is empty and nothing should
    /// be applied.
    bool conflict = false;

    /// Edits on `ours` that add the changes from `theirs`, in increasing
    /// order and in the line numbers of `ours` before any edit is applied
    /// (apply them from the last to the first).
    std::vector<LineEdit> edits;

    /// The lines of `ours` changed by the user that conflict with `theirs`.
    std::vector<LineRange> conflicts;
};

/// Three-way merge like `git merge-file`: `base` is the common ancestor,
/// `ours` the buffer and `theirs` the file on disk. Changes that overlap or
/// touch conflict, unless both sides made the same change.
LineMerge mergeLines(std::string_view base,
                     std::string_view ours,
                     std::string_view theirs);

/// `text` with `edits` (as from `mergeLines`) applied.
std::string applyLineEdits(std::string_view text,
                           const std::vector<LineEdit> &edits);

/// A unified diff from `from` to `to` with `context` lines around changes,
/// without the file header. Empty when the texts are equal.
std::string unifiedDiff(std::string_view from,
                        std::string_view to,
                        size_t context = 3);
