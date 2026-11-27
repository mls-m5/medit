#include "text/fuzzymatch.h"
#include "text/utf8caseconversion.h"
#include <algorithm>
#include <limits>

namespace {

constexpr int startBonus = 8;
constexpr int wordStartBonus = 6;
constexpr int consecutiveBonus = 5;
constexpr int emphasisBonus = 2;
constexpr int gapPenalty = 1;
constexpr int maxLeadingPenalty = 5;
constexpr int prefixBonus = 20;
constexpr int exactBonus = 100;
constexpr int impossible = std::numeric_limits<int>::min() / 2;

bool isSeparator(Utf8Char c) {
    return c == ' ' || c == '_' || c == '-' || c == '/' || c == '.' ||
           c == '\\' || c == ':';
}

bool isIgnoredInPattern(Utf8Char c) {
    return c == ' ' || c == '_';
}

/// The candidate without the characters a pattern ignores, for comparing
/// exact and prefix matches.
std::vector<Utf8Char> stripped(const std::vector<Utf8Char> &text) {
    auto ret = std::vector<Utf8Char>{};
    for (auto c : text) {
        if (!isIgnoredInPattern(c)) {
            ret.push_back(c);
        }
    }
    return ret;
}

} // namespace

std::optional<FuzzyMatch> fuzzyMatch(const FString &pattern,
                                     const FString &candidate,
                                     size_t emphasisStart) {
    auto p = std::vector<Utf8Char>{};
    for (auto &c : pattern) {
        if (!isIgnoredInPattern(c.c)) {
            p.push_back(toLower(c.c));
        }
    }
    if (p.empty()) {
        return FuzzyMatch{};
    }

    const auto n = candidate.size();
    const auto m = p.size();
    auto lower = std::vector<Utf8Char>{};
    lower.reserve(n);
    for (auto &c : candidate) {
        lower.push_back(toLower(c.c));
    }

    // Cheap rejection before scoring: is the pattern a subsequence at all?
    {
        size_t i = 0;
        for (size_t j = 0; j < n && i < m; ++j) {
            if (lower[j] == p[i]) {
                ++i;
            }
        }
        if (i < m) {
            return std::nullopt;
        }
    }

    auto charScore = [&](size_t j) {
        auto score = 1;
        if (j == 0) {
            score += startBonus;
        }
        else {
            const auto previous = candidate.at(j - 1).c;
            const auto current = candidate.at(j).c;
            if (isSeparator(previous) ||
                (toLower(previous) == previous && toLower(current) != current &&
                 !isSeparator(current))) {
                score += wordStartBonus;
            }
        }
        if (j >= emphasisStart) {
            score += emphasisBonus;
        }
        return score;
    };

    // best[i * n + j]: best score with pattern[i] matched at candidate[j];
    // from[i * n + j]: where pattern[i - 1] was matched for that score.
    auto best = std::vector<int>(m * n, impossible);
    auto from = std::vector<size_t>(m * n, 0);

    for (size_t j = 0; j < n; ++j) {
        if (lower[j] == p[0]) {
            best[j] =
                charScore(j) - std::min(static_cast<int>(j), maxLeadingPenalty);
        }
    }

    for (size_t i = 1; i < m; ++i) {
        // Best previous match before j, with the gap up to j already
        // subtracted.
        auto running = impossible;
        auto runningFrom = size_t{0};
        for (size_t j = 1; j < n; ++j) {
            const auto previous = best[(i - 1) * n + (j - 1)];
            if (running != impossible) {
                running -= gapPenalty;
            }
            if (lower[j] != p[i]) {
                if (previous > running) {
                    running = previous;
                    runningFrom = j - 1;
                }
                continue;
            }
            auto score = impossible;
            auto source = size_t{0};
            if (previous != impossible) {
                score = previous + consecutiveBonus;
                source = j - 1;
            }
            if (running != impossible && running > score) {
                score = running;
                source = runningFrom;
            }
            if (score != impossible) {
                best[i * n + j] = score + charScore(j);
                from[i * n + j] = source;
            }
            if (previous > running) {
                running = previous;
                runningFrom = j - 1;
            }
        }
    }

    auto end = size_t{0};
    auto score = impossible;
    for (size_t j = 0; j < n; ++j) {
        if (best[(m - 1) * n + j] > score) {
            score = best[(m - 1) * n + j];
            end = j;
        }
    }
    if (score == impossible) {
        return std::nullopt;
    }

    auto match = FuzzyMatch{};
    match.positions.resize(m);
    for (size_t i = m, j = end; i-- > 0;) {
        match.positions[i] = j;
        j = from[i * n + j];
    }

    const auto text = stripped(lower);
    if (text == p) {
        score += exactBonus;
    }
    else if (text.size() > p.size() &&
             std::equal(p.begin(), p.end(), text.begin())) {
        score += prefixBonus;
    }
    match.score = score;
    return match;
}

std::vector<RankedMatch> fuzzyRank(
    const FString &pattern,
    const std::vector<FuzzyCandidate> &candidates,
    size_t limit) {
    struct Ranked {
        RankedMatch match;
        size_t depth = 0;
    };
    auto ranked = std::vector<Ranked>{};
    for (size_t i = 0; i < candidates.size(); ++i) {
        auto &candidate = candidates[i];
        if (auto match =
                fuzzyMatch(pattern, candidate.text, candidate.emphasisStart)) {
            const auto depth = static_cast<size_t>(
                std::count_if(candidate.text.begin(),
                              candidate.text.end(),
                              [](const FChar &c) { return c.c == '/'; }));
            ranked.push_back({{i, std::move(*match)}, depth});
        }
    }

    auto better = [&candidates](const Ranked &a, const Ranked &b) {
        if (a.match.match.score != b.match.match.score) {
            return a.match.match.score > b.match.match.score;
        }
        if (a.depth != b.depth) {
            return a.depth < b.depth;
        }
        const auto aSize = candidates[a.match.index].text.size();
        const auto bSize = candidates[b.match.index].text.size();
        if (aSize != bSize) {
            return aSize < bSize;
        }
        return a.match.index < b.match.index;
    };

    if (limit < ranked.size()) {
        std::partial_sort(ranked.begin(),
                          ranked.begin() + static_cast<std::ptrdiff_t>(limit),
                          ranked.end(),
                          better);
        ranked.resize(limit);
    }
    else {
        std::sort(ranked.begin(), ranked.end(), better);
    }

    auto ret = std::vector<RankedMatch>{};
    ret.reserve(ranked.size());
    for (auto &r : ranked) {
        ret.push_back(std::move(r.match));
    }
    return ret;
}
