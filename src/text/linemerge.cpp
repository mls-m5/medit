#include "text/linemerge.h"
#include <algorithm>
#include <utility>

namespace {

using Lines = std::vector<std::string_view>;

struct Match {
    size_t from = 0;
    size_t to = 0;
};

/// Myers' O(ND) diff of `a` and `b`. The matching line pairs in increasing
/// order, or false when more than `maxD` lines differ.
bool myers(const Lines &a,
           size_t aBegin,
           size_t n,
           const Lines &b,
           size_t bBegin,
           size_t m,
           size_t maxEdits,
           std::vector<Match> &matches) {
    const auto maxD = static_cast<long>(std::min(n + m, maxEdits));
    const auto offset = maxD + 1;
    auto v = std::vector<long>(static_cast<size_t>(2 * maxD + 3), 0);
    auto at = [&v, offset](long k) -> long & {
        return v[static_cast<size_t>(k + offset)];
    };
    const auto ln = static_cast<long>(n);
    const auto lm = static_cast<long>(m);

    // trace[d]: v[-d..d] before step d, for walking the path back.
    auto trace = std::vector<std::vector<long>>{};
    auto found = -1L;
    for (long d = 0; d <= maxD && found < 0; ++d) {
        trace.emplace_back(v.begin() + (offset - d),
                           v.begin() + (offset + d + 1));
        for (long k = -d; k <= d; k += 2) {
            auto x = (k == -d || (k != d && at(k - 1) < at(k + 1)))
                         ? at(k + 1)
                         : at(k - 1) + 1;
            auto y = x - k;
            while (x < ln && y < lm &&
                   a[aBegin + static_cast<size_t>(x)] ==
                       b[bBegin + static_cast<size_t>(y)]) {
                ++x;
                ++y;
            }
            at(k) = x;
            if (x >= ln && y >= lm) {
                found = d;
                break;
            }
        }
    }
    if (found < 0) {
        return false;
    }

    auto x = ln;
    auto y = lm;
    auto reversed = std::vector<Match>{};
    for (auto d = found; d > 0; --d) {
        const auto &previous = trace[static_cast<size_t>(d)];
        auto prev = [&previous, d](long k) {
            return previous[static_cast<size_t>(k + d)];
        };
        const auto k = x - y;
        const auto previousK =
            (k == -d || (k != d && prev(k - 1) < prev(k + 1))) ? k + 1 : k - 1;
        const auto previousX = prev(previousK);
        const auto previousY = previousX - previousK;
        // The step itself (one insertion or deletion) happens first, then
        // the diagonal of matches up to (x, y).
        const auto startX = previousK == k + 1 ? previousX : previousX + 1;
        const auto startY = startX - k;
        while (x > startX && y > startY) {
            --x;
            --y;
            reversed.push_back(
                {static_cast<size_t>(x), static_cast<size_t>(y)});
        }
        x = previousX;
        y = previousY;
    }
    while (x > 0 && y > 0) {
        --x;
        --y;
        reversed.push_back({static_cast<size_t>(x), static_cast<size_t>(y)});
    }

    matches.assign(reversed.rbegin(), reversed.rend());
    return true;
}

std::vector<std::string> copyLines(const Lines &lines,
                                   size_t begin,
                                   size_t end) {
    return {lines.begin() + static_cast<std::ptrdiff_t>(begin),
            lines.begin() + static_cast<std::ptrdiff_t>(end)};
}

bool sameLines(const Lines &a,
               size_t aBegin,
               size_t aEnd,
               const Lines &b,
               size_t bBegin,
               size_t bEnd) {
    return aEnd - aBegin == bEnd - bBegin &&
           std::equal(a.begin() + static_cast<std::ptrdiff_t>(aBegin),
                      a.begin() + static_cast<std::ptrdiff_t>(aEnd),
                      b.begin() + static_cast<std::ptrdiff_t>(bBegin));
}

std::string join(const std::vector<std::string> &lines) {
    auto ret = std::string{};
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i) {
            ret += '\n';
        }
        ret += lines[i];
    }
    return ret;
}

} // namespace

std::vector<std::string_view> splitLines(std::string_view text) {
    auto ret = std::vector<std::string_view>{};
    for (size_t start = 0;;) {
        const auto end = text.find('\n', start);
        if (end == std::string_view::npos) {
            ret.push_back(text.substr(start));
            return ret;
        }
        ret.push_back(text.substr(start, end - start));
        start = end + 1;
    }
}

std::vector<DiffHunk> diffLines(const std::vector<std::string_view> &from,
                                const std::vector<std::string_view> &to,
                                size_t maxEdits) {
    auto start = size_t{0};
    while (start < from.size() && start < to.size() &&
           from[start] == to[start]) {
        ++start;
    }
    auto fromEnd = from.size();
    auto toEnd = to.size();
    while (fromEnd > start && toEnd > start &&
           from[fromEnd - 1] == to[toEnd - 1]) {
        --fromEnd;
        --toEnd;
    }
    if (start == fromEnd && start == toEnd) {
        return {};
    }

    const auto n = fromEnd - start;
    const auto m = toEnd - start;
    auto matches = std::vector<Match>{};
    if (n == 0 || m == 0 ||
        !myers(from, start, n, to, start, m, maxEdits, matches)) {
        return {{start, fromEnd, start, toEnd}};
    }

    auto ret = std::vector<DiffHunk>{};
    auto i = size_t{0};
    auto j = size_t{0};
    for (auto match : matches) {
        if (match.from > i || match.to > j) {
            ret.push_back(
                {start + i, start + match.from, start + j, start + match.to});
        }
        i = match.from + 1;
        j = match.to + 1;
    }
    if (i < n || j < m) {
        ret.push_back({start + i, fromEnd, start + j, toEnd});
    }
    return ret;
}

LineMerge mergeLines(std::string_view base,
                     std::string_view ours,
                     std::string_view theirs,
                     size_t margin) {
    const auto baseLines = splitLines(base);
    const auto ourLines = splitLines(ours);
    const auto theirLines = splitLines(theirs);
    const auto ourHunks = diffLines(baseLines, ourLines);
    const auto theirHunks = diffLines(baseLines, theirLines);

    // Overlapping or adjacent ranges in the base, as in `git merge-file`,
    // or closer than `margin` lines.
    const auto extra = margin ? margin - 1 : 0;
    auto touches = [extra](const DiffHunk &a, const DiffHunk &b) {
        return a.fromBegin <= b.fromEnd + extra &&
               b.fromBegin <= a.fromEnd + extra;
    };
    auto same = [&](const DiffHunk &our, const DiffHunk &their) {
        return our.fromBegin == their.fromBegin &&
               our.fromEnd == their.fromEnd &&
               sameLines(ourLines,
                         our.toBegin,
                         our.toEnd,
                         theirLines,
                         their.toBegin,
                         their.toEnd);
    };

    auto ret = LineMerge{};
    for (auto &their : theirHunks) {
        auto shift = long{0}; // From base to our line numbers.
        auto applies = true;
        for (auto &our : ourHunks) {
            if (touches(our, their)) {
                if (same(our, their)) {
                    applies = false; // Already in ours.
                    continue;
                }
                ret.conflict = true;
                applies = false;
                const auto range = LineRange{our.toBegin, our.toEnd};
                if (std::find(ret.conflicts.begin(),
                              ret.conflicts.end(),
                              range) == ret.conflicts.end()) {
                    ret.conflicts.push_back(range);
                }
            }
            else if (our.fromEnd < their.fromBegin) {
                shift += static_cast<long>(our.toEnd - our.toBegin) -
                         static_cast<long>(our.fromEnd - our.fromBegin);
            }
        }
        if (applies) {
            ret.edits.push_back({
                static_cast<size_t>(static_cast<long>(their.fromBegin) + shift),
                static_cast<size_t>(static_cast<long>(their.fromEnd) + shift),
                copyLines(theirLines, their.toBegin, their.toEnd),
            });
        }
    }

    if (ret.conflict) {
        ret.edits.clear();
        std::sort(ret.conflicts.begin(),
                  ret.conflicts.end(),
                  [](const LineRange &a, const LineRange &b) {
                      return a.begin < b.begin;
                  });
    }
    return ret;
}

std::string applyLineEdits(std::string_view text,
                           const std::vector<LineEdit> &edits) {
    const auto views = splitLines(text);
    auto lines = std::vector<std::string>{views.begin(), views.end()};
    for (auto it = edits.rbegin(); it != edits.rend(); ++it) {
        const auto begin =
            lines.begin() + static_cast<std::ptrdiff_t>(it->begin);
        lines.erase(begin,
                    lines.begin() + static_cast<std::ptrdiff_t>(it->end));
        lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(it->begin),
                     it->lines.begin(),
                     it->lines.end());
    }
    return join(lines);
}

std::string unifiedDiff(std::string_view from,
                        std::string_view to,
                        size_t context) {
    const auto a = splitLines(from);
    const auto b = splitLines(to);
    const auto hunks = diffLines(a, b);

    auto ret = std::string{};
    auto range = [](size_t begin, size_t length) {
        // As in diff: an empty range names the line before it.
        return std::to_string(length ? begin + 1 : begin) + "," +
               std::to_string(length);
    };

    for (size_t first = 0; first < hunks.size();) {
        auto last = first;
        while (last + 1 < hunks.size() &&
               hunks[last + 1].fromBegin - hunks[last].fromEnd <= 2 * context) {
            ++last;
        }
        const auto &h0 = hunks[first];
        const auto &h1 = hunks[last];
        const auto lead = std::min(context, h0.fromBegin);
        const auto trail = std::min(context, a.size() - h1.fromEnd);
        const auto fromBegin = h0.fromBegin - lead;
        const auto fromEnd = h1.fromEnd + trail;
        const auto toBegin = h0.toBegin - lead;
        const auto toEnd = h1.toEnd + trail;

        ret += "@@ -" + range(fromBegin, fromEnd - fromBegin) + " +" +
               range(toBegin, toEnd - toBegin) + " @@\n";
        auto line = fromBegin;
        for (auto h = first; h <= last; ++h) {
            for (; line < hunks[h].fromBegin; ++line) {
                ret += " ";
                ret += a[line];
                ret += "\n";
            }
            for (auto i = hunks[h].fromBegin; i < hunks[h].fromEnd; ++i) {
                ret += "-";
                ret += a[i];
                ret += "\n";
            }
            for (auto i = hunks[h].toBegin; i < hunks[h].toEnd; ++i) {
                ret += "+";
                ret += b[i];
                ret += "\n";
            }
            line = hunks[h].fromEnd;
        }
        for (; line < fromEnd; ++line) {
            ret += " ";
            ret += a[line];
            ret += "\n";
        }
        first = last + 1;
    }
    return ret;
}
