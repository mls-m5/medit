#include "mls-unit-test/unittest.h"
#include "text/fuzzymatch.h"
#include <string>
#include <vector>

namespace fuzzy_match_test {

std::vector<std::string> rank(const std::string &pattern,
                              const std::vector<std::string> &texts) {
    auto candidates = std::vector<FuzzyCandidate>{};
    for (auto &text : texts) {
        auto candidate = FuzzyCandidate{FString{text}, 0};
        if (auto slash = text.rfind('/'); slash != std::string::npos) {
            candidate.emphasisStart = slash + 1; // The file name.
        }
        candidates.push_back(std::move(candidate));
    }
    auto ret = std::vector<std::string>{};
    for (auto &r : fuzzyRank(FString{pattern}, candidates)) {
        ret.push_back(texts.at(r.index));
    }
    return ret;
}

} // namespace fuzzy_match_test

TEST_SUIT_BEGIN(FuzzyMatch)

TEST_CASE("characters match in order anywhere, ignoring case") {
    EXPECT_TRUE(fuzzyMatch(FString{"gcm"}, FString{"git_commit_amend"}));
    EXPECT_TRUE(fuzzyMatch(FString{"GCA"}, FString{"git_commit_amend"}));
    EXPECT_TRUE(fuzzyMatch(FString{"mit"}, FString{"git_commit"}));
    EXPECT_FALSE(fuzzyMatch(FString{"mcg"}, FString{"git_commit"}));
    EXPECT_FALSE(fuzzyMatch(FString{"x"}, FString{"git_commit"}));

    const auto empty = fuzzyMatch(FString{""}, FString{"anything"});
    ASSERT_EQ(empty.has_value(), true);
    ASSERT_EQ(empty->score, 0);
}

TEST_CASE("spaces and underscores in the pattern are ignored") {
    EXPECT_TRUE(fuzzyMatch(FString{"git commit"}, FString{"git_commit"}));
    EXPECT_TRUE(fuzzyMatch(FString{"git_commit"}, FString{"git commit"}));
}

TEST_CASE("positions point at word starts when possible") {
    const auto match = fuzzyMatch(FString{"gca"}, FString{"git_commit_amend"});
    ASSERT_EQ(match.has_value(), true);
    ASSERT_EQ(match->positions.size(), 3u);
    ASSERT_EQ(match->positions[0], 0u);
    ASSERT_EQ(match->positions[1], 4u); // c in commit, not the first 'c'.
    ASSERT_EQ(match->positions[2], 11u);

    const auto camel = fuzzyMatch(FString{"fm"}, FString{"fuzzyMatch.cpp"});
    ASSERT_EQ(camel->positions[1], 5u);
}

TEST_CASE("non-ASCII names match per character") {
    const auto match = fuzzyMatch(FString{"åö"}, FString{"bår/mjöl.txt"});
    ASSERT_EQ(match.has_value(), true);
    ASSERT_EQ(match->positions[0], 1u);
    ASSERT_EQ(match->positions[1], 6u);
}

TEST_CASE("exact and prefix matches rank first") {
    using Texts = std::vector<std::string>;
    EXPECT_TRUE(fuzzy_match_test::rank("git commit",
                                       {"git_commit_amend", "git_commit"}) ==
                (Texts{"git_commit", "git_commit_amend"}));
    EXPECT_TRUE(fuzzy_match_test::rank("run", {"debug_run", "run", "rename"}) ==
                (Texts{"run", "debug_run"}));
    EXPECT_TRUE(fuzzy_match_test::rank("save", {"save_as", "autosave", "save"})
                    .front() == "save");
}

TEST_CASE("word starts and consecutive characters beat scattered ones") {
    const auto ranked =
        fuzzy_match_test::rank("gd", {"git_add_old", "git_diff"});
    ASSERT_EQ(ranked.front(), "git_diff");

    const auto files = fuzzy_match_test::rank(
        "quick", {"src/views/sidebar.cpp", "src/views/quicklist.cpp"});
    ASSERT_EQ(files.size(), 1u);
    ASSERT_EQ(files.front(), "src/views/quicklist.cpp");
}

TEST_CASE("the file name is preferred over directories") {
    const auto ranked = fuzzy_match_test::rank(
        "view", {"src/views/editor.cpp", "src/text/view.cpp"});
    ASSERT_EQ(ranked.front(), "src/text/view.cpp");
}

TEST_CASE("equal matches closer to the root come first") {
    const auto ranked = fuzzy_match_test::rank(
        "readme", {"docs/deep/README.md", "docs/README.md", "README.md"});
    ASSERT_EQ(ranked.size(), 3u);
    ASSERT_EQ(ranked[0], "README.md");
    ASSERT_EQ(ranked[1], "docs/README.md");
    ASSERT_EQ(ranked[2], "docs/deep/README.md");
}

TEST_CASE("the limit keeps only the best matches") {
    auto candidates = std::vector<FuzzyCandidate>{};
    for (auto text : {"axxxxb", "ab", "axb"}) {
        candidates.push_back({FString{text}, 0});
    }
    const auto ranked = fuzzyRank(FString{"ab"}, candidates, 2);
    ASSERT_EQ(ranked.size(), 2u);
    ASSERT_EQ(ranked[0].index, 1u);
    ASSERT_EQ(ranked[1].index, 2u);
}

TEST_SUIT_END
