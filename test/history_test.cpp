#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "text/history.h"
#include <string_view>
#include <vector>

using namespace std::literals;

namespace history_test {

std::vector<HistoryKind> kinds(const History &history) {
    auto ret = std::vector<HistoryKind>{};
    for (auto &item : history.items()) {
        ret.push_back(item.kind);
    }
    return ret;
}

} // namespace history_test

TEST_SUIT_BEGIN(History)

TEST_CASE("simple undo") {
    auto testString1 = "hello there"s;
    auto testString2 = "wazzup?"s;

    auto buffer = Buffer{testString1};
    auto &history = buffer.history();

    buffer.text(testString2);

    history.undo();

    ASSERT_EQ(buffer.text(), testString1);

    history.redo();

    ASSERT_EQ(buffer.text(), testString2);
}

TEST_CASE("simple undo") {
    auto testString1 = "hello there"s;
    auto testString2 = "wazzup?"s;
    auto testString3 = "wazzupzz?"s;

    auto buffer = Buffer{testString1};
    auto &history = buffer.history();

    history.markMajor();

    buffer.text(testString2);

    history.markMajor();

    buffer.text(testString3);

    history.markMajor();

    history.undoMajor();

    ASSERT_EQ(buffer.text(), testString2);

    history.undoMajor();

    ASSERT_EQ(buffer.text(), testString1);

    history.redo();

    ASSERT_EQ(buffer.text(), testString2);

    history.undo();

    ASSERT_EQ(buffer.text(), testString1);
}

TEST_CASE("multi line undo") {
    auto testString1 = "apa\nbepa\ncepa\ndepa"s;

    auto buffer = Buffer{testString1};
    auto &history = buffer.history();

    history.markMajor();

    buffer.text("apa\nbe2pa\nc234epa\ndepa"s);

    history.markMajor();

    history.undoMajor();

    ASSERT_EQ(buffer.text(), testString1);
}

TEST_CASE("markers are recorded and passed over by undo and redo") {
    auto buffer = Buffer{"a"};
    auto &history = buffer.history();
    buffer.text("b"s);
    history.markMajor();
    history.event(HistoryKind::Save);
    buffer.text("c"s);
    history.markMajor();

    using K = HistoryKind;
    EXPECT_TRUE((history_test::kinds(history) ==
                 std::vector{K::Edit, K::Edit, K::Save, K::Edit}));

    history.undo();
    ASSERT_EQ(buffer.text(), "b");
    // The save happened after "b", so it is still there.
    EXPECT_TRUE((history_test::kinds(history) ==
                 std::vector{K::Edit, K::Edit, K::Save}));

    history.undo();
    ASSERT_EQ(buffer.text(), "a");
    // Undone together with the edit before it...
    EXPECT_TRUE((history_test::kinds(history) == std::vector{K::Edit}));

    history.redo();
    ASSERT_EQ(buffer.text(), "b");
    // ...and redone with it.
    EXPECT_TRUE((history_test::kinds(history) ==
                 std::vector{K::Edit, K::Edit, K::Save}));

    history.redo();
    ASSERT_EQ(buffer.text(), "c");
}

TEST_CASE("undoMajor stops at a major edit even after a marker") {
    auto buffer = Buffer{"a"};
    auto &history = buffer.history();
    buffer.text("b"s);
    history.markMajor();
    buffer.text("c"s);
    history.event(HistoryKind::Save);
    buffer.text("d"s);
    history.markMajor();

    history.undoMajor();
    ASSERT_EQ(buffer.text(), "b");
    // A marker is never marked major instead of the edit before it.
    history.event(HistoryKind::Save);
    history.markMajor();
    history.undoMajor();
    ASSERT_EQ(buffer.text(), "a");
}

TEST_CASE("edits can be tagged by what made them") {
    auto buffer = Buffer{"a"};
    auto &history = buffer.history();
    const auto revision = history.revision();
    buffer.text("b"s);
    buffer.text("c"s);
    history.tagSince(revision, HistoryKind::Reload);
    using K = HistoryKind;
    EXPECT_TRUE((history_test::kinds(history) ==
                 std::vector{K::Edit, K::Reload, K::Reload}));
}

TEST_SUIT_END
