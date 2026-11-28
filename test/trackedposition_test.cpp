#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "text/bufferedit.h"
#include "text/trackedposition.h"
#include "views/editor.h"
#include <memory>

namespace tracked_position_test {

/// An edit replacing `from` with `to` at (x, y), as applied (trimmed).
BufferEdit edit(
    Buffer &buffer, size_t x, size_t y, const char *from, const char *to) {
    return BufferEdit{FString{from}, FString{to}, Cursor{buffer, x, y}};
}

} // namespace tracked_position_test

TEST_SUIT_BEGIN(TrackedPosition)

TEST_CASE("positions move with the text around them") {
    auto buffer = Buffer{};
    using tracked_position_test::edit;
    auto insert = edit(buffer, 2, 1, "", "abc");
    // Before: unchanged. Exactly at the insertion: after the new text.
    EXPECT_TRUE((movePosition({5, 0}, insert) == Position{5, 0}));
    EXPECT_TRUE((movePosition({2, 1}, insert) == Position{5, 1}));
    // At the start of a replacement: unchanged.
    EXPECT_TRUE((movePosition({2, 1}, edit(buffer, 2, 1, "x", "yz")) ==
                 Position{2, 1}));
    // After it on the same line: moved right; on later lines: unchanged.
    EXPECT_TRUE((movePosition({4, 1}, insert) == Position{7, 1}));
    EXPECT_TRUE((movePosition({4, 3}, insert) == Position{4, 3}));

    auto newlines = edit(buffer, 2, 1, "", "a\nb\ncd");
    EXPECT_TRUE((movePosition({4, 1}, newlines) == Position{4, 3}));
    EXPECT_TRUE((movePosition({0, 2}, newlines) == Position{0, 4}));

    auto removal = edit(buffer, 1, 1, "xy\nzw", "");
    // After the removed text: joined onto the line where it started.
    EXPECT_TRUE((movePosition({3, 2}, removal) == Position{2, 1}));
    EXPECT_TRUE((movePosition({0, 5}, removal) == Position{0, 4}));
    // Inside it: kept, but not after the inserted (here empty) text.
    EXPECT_TRUE((movePosition({0, 2}, removal) == Position{1, 1}));

    auto replace = edit(buffer, 0, 1, "old\nline", "new\nlines\nhere");
    EXPECT_TRUE((movePosition({2, 2}, replace) == Position{2, 2}));
    EXPECT_TRUE((movePosition({4, 2}, replace) == Position{4, 3}));
}

TEST_CASE("tracked positions follow edits, undo and reloads") {
    auto buffer = std::make_shared<Buffer>("a\nb\nc\n");
    auto position = TrackedPosition{*buffer, {0, 2}}; // On "c".

    buffer->apply(BufferEdit{FString{""}, FString{"new\n"}, buffer->begin()});
    EXPECT_TRUE((position.pos() == Position{0, 3}));

    buffer->history().undo();
    EXPECT_TRUE((position.pos() == Position{0, 2}));
    buffer->history().redo();
    EXPECT_TRUE((position.pos() == Position{0, 3}));

    // A reload that inserts two lines at the top keeps it on "c".
    buffer->replaceText("x\ny\nnew\na\nb\nc\n");
    EXPECT_TRUE((position.pos() == Position{0, 5}));
    EXPECT_TRUE(buffer->lineAt(position.pos().y()).front().c == Utf8Char{'c'});
}

TEST_CASE("a tracked position outlives its buffer and releases its anchor") {
    auto buffer = std::make_shared<Buffer>("a\nb\n");
    auto position = std::make_unique<TrackedPosition>(*buffer, Position{0, 1});
    auto moved = std::move(*position);
    position.reset(); // The moved-from one must not remove the anchor.
    buffer->apply(BufferEdit{FString{""}, FString{"x\n"}, buffer->begin()});
    EXPECT_TRUE((moved.pos() == Position{0, 2}));
    buffer.reset();
    EXPECT_TRUE((moved.pos() == Position{0, 2}));
    EXPECT_TRUE(moved.buffer() == nullptr);
}

TEST_CASE("an editor's cursor and selection follow changes from elsewhere") {
    auto buffer = std::make_shared<Buffer>("one\ntwo\nthree\n");
    auto editor = Editor{nullptr, buffer};
    editor.selectionAnchor(Cursor{*buffer, 0, 1});
    editor.cursor(Cursor{*buffer, 2, 2}); // "three", on the 'r'.

    // Someone else (a reload, another editor) changes lines above.
    buffer->replaceText("zero\none\none and a half\ntwo\nthree\n");
    EXPECT_TRUE((editor.cursor().pos() == Position{2, 4}));
    EXPECT_TRUE((editor.selectionAnchor()->pos() == Position{0, 3}));
}

TEST_SUIT_END
