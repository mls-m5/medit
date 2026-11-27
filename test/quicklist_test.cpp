#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "views/quicklist.h"
#include <string>
#include <vector>

namespace quick_list_test {

using Values = std::vector<std::string>;

QuickList::PopulateRetT items(const Values &values) {
    auto ret = QuickList::PopulateRetT{};
    for (auto &value : values) {
        ret.push_back({FString{value}, value, 0, {}});
    }
    return ret;
}

Values search(QuickList &list, const std::string &text) {
    list.buffer().text(text);
    list.filter();
    return list.shownValues();
}

} // namespace quick_list_test

TEST_SUIT_BEGIN(QuickList)

TEST_CASE("typed text filters and ranks, empty text keeps the given order") {
    using quick_list_test::Values;
    auto list = QuickList{
        nullptr, [] {
            return quick_list_test::items(
                {"debug_run", "git_commit_amend", "git_commit", "run"});
        }};
    list.visible(true);
    EXPECT_TRUE(list.shownValues() ==
                (Values{"debug_run", "git_commit_amend", "git_commit", "run"}));
    EXPECT_TRUE(quick_list_test::search(list, "git commit") ==
                (Values{"git_commit", "git_commit_amend"}));
    EXPECT_TRUE(quick_list_test::search(list, "run").front() == "run");
    EXPECT_TRUE(quick_list_test::search(list, "gca") ==
                (Values{"git_commit_amend"}));
    EXPECT_TRUE(quick_list_test::search(list, "xyz").empty());
    EXPECT_TRUE(quick_list_test::search(list, "  ").size() == 4u);
}

TEST_CASE("the list is read when shown, not on every key") {
    auto reads = 0;
    auto list = QuickList{nullptr, [&reads] {
                              ++reads;
                              return quick_list_test::items({"a", "b"});
                          }};
    list.visible(true);
    quick_list_test::search(list, "a");
    quick_list_test::search(list, "b");
    ASSERT_EQ(reads, 1);
    list.visible(true);
    ASSERT_EQ(reads, 2);
}

TEST_CASE("at most maxShown rows are shown") {
    auto values = quick_list_test::Values{};
    for (size_t i = 0; i < QuickList::maxShown + 50; ++i) {
        values.push_back("item" + std::to_string(i));
    }
    auto list = QuickList{nullptr,
                          [&values] { return quick_list_test::items(values); }};
    list.visible(true);
    ASSERT_EQ(list.shownValues().size(), QuickList::maxShown);
    ASSERT_EQ(quick_list_test::search(list, "item").size(),
              QuickList::maxShown);
}

TEST_SUIT_END
