#include "mls-unit-test/unittest.h"
#include "text/linemerge.h"
#include <random>
#include <string>
#include <vector>

namespace line_merge_test {

/// `from` with `hunks` applied, to check that a diff reproduces `to`.
std::string patch(std::string_view from,
                  std::string_view to,
                  const std::vector<DiffHunk> &hunks) {
    const auto b = splitLines(to);
    auto edits = std::vector<LineEdit>{};
    for (auto &h : hunks) {
        edits.push_back({h.fromBegin,
                         h.fromEnd,
                         {b.begin() + static_cast<std::ptrdiff_t>(h.toBegin),
                          b.begin() + static_cast<std::ptrdiff_t>(h.toEnd)}});
    }
    return applyLineEdits(from, edits);
}

std::vector<DiffHunk> diff(std::string_view from, std::string_view to) {
    return diffLines(splitLines(from), splitLines(to));
}

/// The merged text, or "CONFLICT".
std::string merged(std::string_view base,
                   std::string_view ours,
                   std::string_view theirs) {
    const auto merge = mergeLines(base, ours, theirs);
    if (merge.conflict) {
        return "CONFLICT";
    }
    return applyLineEdits(ours, merge.edits);
}

} // namespace line_merge_test

TEST_SUIT_BEGIN(LineMerge)

TEST_CASE("lines are split on newlines only") {
    EXPECT_TRUE((splitLines("") == std::vector<std::string_view>{""}));
    EXPECT_TRUE((splitLines("a\r\nb\n") ==
                 std::vector<std::string_view>{"a\r", "b", ""}));
}

TEST_CASE("diffs find the changed lines") {
    EXPECT_TRUE(line_merge_test::diff("a\nb\nc", "a\nb\nc").empty());

    EXPECT_TRUE((line_merge_test::diff("a\nb\nc", "a\nB\nc") ==
                 std::vector<DiffHunk>{{1, 2, 1, 2}}));
    EXPECT_TRUE((line_merge_test::diff("a\nc", "a\nb\nc") ==
                 std::vector<DiffHunk>{{1, 1, 1, 2}}));
    EXPECT_TRUE((line_merge_test::diff("a\nb\nc", "a\nc") ==
                 std::vector<DiffHunk>{{1, 2, 1, 1}}));
    // Two separate changes give two hunks, not one spanning both.
    EXPECT_TRUE((line_merge_test::diff("1\n2\n3\n4\n5", "x\n2\n3\n4\ny") ==
                 std::vector<DiffHunk>{{0, 1, 0, 1}, {4, 5, 4, 5}}));
}

TEST_CASE("random diffs reproduce the new text") {
    auto random = std::mt19937{1234};
    auto text = [&random] {
        auto ret = std::string{};
        const auto lines = random() % 12;
        for (size_t i = 0; i < lines; ++i) {
            ret += static_cast<char>('a' + random() % 4);
            ret += '\n';
        }
        return ret;
    };
    for (int i = 0; i < 2000; ++i) {
        const auto from = text();
        const auto to = text();
        const auto hunks = line_merge_test::diff(from, to);
        ASSERT_EQ(line_merge_test::patch(from, to, hunks), to);
        // The diff must be minimal enough that equal texts give nothing.
        EXPECT_TRUE(line_merge_test::diff(from, from).empty());
    }
}

TEST_CASE("a diff that is too large becomes one hunk and is still right") {
    auto from = std::string{};
    auto to = std::string{};
    for (int i = 0; i < 50; ++i) {
        // Appended piece by piece: `"a" + std::to_string(i)` gives a false
        // -Wrestrict warning with GCC 12.
        from += 'a';
        from += std::to_string(i);
        from += '\n';
        to += 'b';
        to += std::to_string(i);
        to += '\n';
    }
    const auto hunks = diffLines(splitLines(from), splitLines(to), 10);
    ASSERT_EQ(hunks.size(), 1u);
    ASSERT_EQ(line_merge_test::patch(from, to, hunks), to);
}

TEST_CASE("changes in different places merge") {
    const auto base = "1\n2\n3\n4\n5\n6\n7\n";
    // Ours edits the start, theirs the end.
    ASSERT_EQ(line_merge_test::merged(
                  base, "one\n2\n3\n4\n5\n6\n7\n", "1\n2\n3\n4\n5\n6\nseven\n"),
              "one\n2\n3\n4\n5\n6\nseven\n");
    // Line numbers shift when ours inserts lines before their change.
    ASSERT_EQ(line_merge_test::merged(base,
                                      "0a\n0b\n1\n2\n3\n4\n5\n6\n7\n",
                                      "1\n2\n3\n4\n5\nsix\n7\n"),
              "0a\n0b\n1\n2\n3\n4\n5\nsix\n7\n");
    // Theirs deletes and inserts while ours deletes elsewhere.
    ASSERT_EQ(line_merge_test::merged(
                  base, "1\n2\n3\n4\n5\n7\n", "1\nnew\n2\n4\n5\n6\n7\n"),
              "1\nnew\n2\n4\n5\n7\n");
}

TEST_CASE("nothing to merge when only one side changed") {
    const auto base = "a\nb\n";
    ASSERT_EQ(line_merge_test::merged(base, base, "a\nB\n"), "a\nB\n");
    ASSERT_EQ(line_merge_test::merged(base, "A\nb\n", base), "A\nb\n");
    EXPECT_TRUE(mergeLines(base, "A\nb\n", base).edits.empty());
}

TEST_CASE("the same change on both sides is not a conflict") {
    const auto base = "a\nb\nc\nd\ne\n";
    ASSERT_EQ(
        line_merge_test::merged(base, "a\nB\nc\nd\ne\n", "a\nB\nc\nd\nE\n"),
        "a\nB\nc\nd\nE\n");
}

TEST_CASE("overlapping and adjacent changes conflict") {
    const auto base = "a\nb\nc\nd\n";
    // The same line changed differently.
    const auto same = mergeLines(base, "a\nX\nc\nd\n", "a\nY\nc\nd\n");
    EXPECT_TRUE(same.conflict);
    EXPECT_TRUE(same.edits.empty());
    EXPECT_TRUE((same.conflicts == std::vector<LineRange>{{1, 2}}));

    // Neighbouring lines, as in git merge-file.
    EXPECT_TRUE(mergeLines(base, "a\nX\nc\nd\n", "a\nb\nY\nd\n").conflict);

    // Insertions at the same place.
    EXPECT_TRUE(
        mergeLines(base, "a\nb\nX\nc\nd\n", "a\nb\nY\nc\nd\n").conflict);

    // One side deletes a line the other side changed.
    const auto deleted = mergeLines(base, "a\nc\nd\n", "a\nY\nc\nd\n");
    EXPECT_TRUE(deleted.conflict);
    EXPECT_TRUE((deleted.conflicts == std::vector<LineRange>{{1, 1}}));
}

TEST_CASE("conflicts report every conflicting range of ours") {
    const auto base = "1\n2\n3\n4\n5\n6\n7\n8\n9\n";
    const auto merge = mergeLines(
        base, "1\nX\n3\n4\n5\n6\n7\nZ\n9\n", "1\nY\n3\n4\n5\n6\n7\nW\n9\n");
    EXPECT_TRUE(merge.conflict);
    EXPECT_TRUE((merge.conflicts == std::vector<LineRange>{{1, 2}, {7, 8}}));
}

TEST_CASE("edits at the start, at the end and on empty texts") {
    ASSERT_EQ(line_merge_test::merged("", "", "new"), "new");
    ASSERT_EQ(line_merge_test::merged("a\nb\nc\nd", "A\nb\nc\nd", "a\nb\nc\nD"),
              "A\nb\nc\nD");
    // A missing final newline is a change of the last line.
    ASSERT_EQ(
        line_merge_test::merged("a\nb\nc\nd\n", "A\nb\nc\nd\n", "a\nb\nc\nd"),
        "A\nb\nc\nd");
    // CRLF line endings are kept as they are.
    ASSERT_EQ(line_merge_test::merged("a\r\nb\r\nc\r\nd\r\n",
                                      "A\r\nb\r\nc\r\nd\r\n",
                                      "a\r\nb\r\nc\r\nD\r\n"),
              "A\r\nb\r\nc\r\nD\r\n");
}

TEST_CASE("a margin makes changes near each other conflict") {
    const auto base = "1\n2\n3\n4\n5\n6\n7\n8\n";
    // Lines 1 and 5 changed: three unchanged lines (2-4) between them.
    const auto ours = "one\n2\n3\n4\n5\n6\n7\n8\n";
    const auto theirs = "1\n2\n3\n4\nfive\n6\n7\n8\n";
    EXPECT_FALSE(mergeLines(base, ours, theirs, 3).conflict);
    EXPECT_TRUE(mergeLines(base, ours, theirs, 4).conflict);
    // Two unchanged lines between: a conflict with margin 3, not without.
    const auto closer = "1\n2\n3\nfour\n5\n6\n7\n8\n";
    EXPECT_TRUE(mergeLines(base, ours, closer, 3).conflict);
    EXPECT_FALSE(mergeLines(base, ours, closer).conflict);
    EXPECT_TRUE((mergeLines(base, ours, closer, 3).conflicts ==
                 std::vector<LineRange>{{0, 1}}));
}

TEST_CASE("unified diffs with context") {
    ASSERT_EQ(unifiedDiff("a\nb", "a\nb"), "");
    ASSERT_EQ(unifiedDiff("1\n2\n3\n4\n5\n6\n7\n8\n9\n10",
                          "1\n2\n3\n4\nfive\n6\n7\n8\n9\n10",
                          2),
              "@@ -3,5 +3,5 @@\n 3\n 4\n-5\n+five\n 6\n 7\n");
    // Changes closer than twice the context share a hunk.
    ASSERT_EQ(unifiedDiff("1\n2\n3\n4\n5", "x\n2\n3\n4\ny", 2),
              "@@ -1,5 +1,5 @@\n-1\n+x\n 2\n 3\n 4\n-5\n+y\n");
    ASSERT_EQ(unifiedDiff("1\n2\n3\n4\n5\n6", "x\n2\n3\n4\n5\ny", 1),
              "@@ -1,2 +1,2 @@\n-1\n+x\n 2\n@@ -5,2 +5,2 @@\n 5\n-6\n+y\n");
    // Insertion into an empty text.
    ASSERT_EQ(unifiedDiff("", "a", 3), "@@ -1,1 +1,1 @@\n-\n+a\n");
}

TEST_SUIT_END
