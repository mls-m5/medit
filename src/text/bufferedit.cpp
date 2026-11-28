#include "bufferedit.h"

void BufferEdit::trim() {
    for (; !to.empty() && !from.empty();) {
        if (to.back() == from.back()) {
            to.pop_back();
            from.pop_back();
            continue;
        }
        break;
    }

    auto minLen = std::min(to.size(), from.size());

    size_t erasedChars = 0;

    for (size_t i = 0; i < minLen; ++i) {
        if (to.at(i) == from.at(i)) {
            position += to.at(i).c.front();
            //            ++position;
            ++erasedChars;
            continue;
        }
        break;
    }

    to.erase(0, erasedChars);
    from.erase(0, erasedChars);
}

BufferEdit revert(BufferEdit e) {
    std::swap(e.from, e.to);
    return e;
}

namespace {

/// The position after `text` written from `start`.
Position endOf(Position start, const FString &text) {
    auto lines = size_t{0};
    auto lastLineStart = size_t{0};
    for (size_t i = 0; i < text.size(); ++i) {
        if (text.at(i) == Utf8Char{'\n'}) {
            ++lines;
            lastLineStart = i + 1;
        }
    }
    if (!lines) {
        return {start.x() + text.size(), start.y()};
    }
    return {text.size() - lastLineStart, start.y() + lines};
}

} // namespace

Position movePosition(Position position, const BufferEdit &edit) {
    const auto begin = edit.position.pos();
    if (position < begin) {
        return position;
    }
    const auto insertedEnd = endOf(begin, edit.to);
    if (position == begin) {
        // Text inserted at the position goes before it (so lines inserted
        // above keep it on its line); a replacement starting here keeps it.
        return edit.from.empty() ? insertedEnd : position;
    }
    const auto removedEnd = endOf(begin, edit.from);
    if (!(position < removedEnd)) {
        if (position.y() == removedEnd.y()) {
            return {insertedEnd.x() + (position.x() - removedEnd.x()),
                    insertedEnd.y()};
        }
        return {position.x(), position.y() - removedEnd.y() + insertedEnd.y()};
    }
    if (insertedEnd < position) {
        return insertedEnd;
    }
    return position;
}
