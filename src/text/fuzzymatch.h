#pragma once

#include "text/fstring.h"
#include <cstddef>
#include <optional>
#include <vector>

/// A match as in VS Code's quick open: the characters of the pattern appear in
/// order anywhere in the candidate, e.g. "gcm" in "git_commit_amend".
struct FuzzyMatch {
    int score = 0;                 // Higher is better
    std::vector<size_t> positions; // Matched characters of the candidate
};

/// Case-insensitive; spaces and '_' in the pattern are ignored, so "git commit"
/// and "git_commit" mean the same. Matches at the start, at word starts (after
/// a separator or a lower-to-upper case change) and in a row score higher;
/// skipped characters lower the score. Characters from `emphasisStart` on
/// score a little higher (used for the file name part of a path). An empty
/// pattern matches everything with score 0.
std::optional<FuzzyMatch> fuzzyMatch(const FString &pattern,
                                     const FString &candidate,
                                     size_t emphasisStart = 0);

struct FuzzyCandidate {
    FString text;
    size_t emphasisStart = 0;
};

struct RankedMatch {
    size_t index = 0; // Into the candidates
    FuzzyMatch match;
};

/// The matching candidates, best first, at most `limit` of them. Equal scores
/// are ordered by fewer '/' (closer to the root), then shorter text, then the
/// original order.
std::vector<RankedMatch> fuzzyRank(
    const FString &pattern,
    const std::vector<FuzzyCandidate> &candidates,
    size_t limit = static_cast<size_t>(-1));
