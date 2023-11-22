
#include "cursorops.h"
#include "buffer.h"
#include "cursorrangeops.h"
#include "text/fstring.h"
#include "text/utf8char.h"
#include "text/utf8charops.h"
#include <optional>

namespace {

enum class InternalCharType {
    Space = 0,
    Alnum = 1,
    Other = 2, // Special characters
};

InternalCharType getType(Cursor cursor) {
    auto c = content(cursor).front();
    if (isalnum(c) || c == '_') {
        return InternalCharType::Alnum;
    }
    if (isspace(c)) {
        return InternalCharType::Space;
    }
    return InternalCharType::Other;
};

} // namespace

bool isValid(Cursor cursor) {
    auto &lines = cursor.buffer().lines();
    if (cursor.y() >= lines.size()) {
        return false;
    }
    else if (cursor.x() > lines.at(cursor.y()).size()) {
        return false;
    }
    return true;
}

Cursor fix(Cursor cursor) {
    auto &lines = cursor.buffer().lines();
    if (lines.empty()) {
        return {cursor.buffer(), 0, 0};
    }
    else if (cursor.y() >= lines.size()) {
        cursor.y(lines.size() - 1);
    }

    auto &line = lines.at(cursor.y());
    if (line.empty()) {
        cursor.x(0);
    }
    else if (cursor.x() > line.size()) {
        cursor.x(line.size());
    }
    return cursor;
}

Cursor right(Cursor cursor, bool allowLineChange) {
    auto &lines = cursor.buffer().lines();
    if (lines.empty()) {
        return {cursor.buffer()};
    }

    cursor = fix(cursor);
    auto &line = lines.at(cursor.y());

    if (cursor.x() == line.size()) {
        if (allowLineChange) {
            if (cursor.y() == lines.size() - 1) {
                return cursor;
            }
            cursor.x(0);
            cursor.y(cursor.y() + 1);
        }
    }
    else {
        cursor.x(cursor.x() + 1);
    }

    return cursor;
}

Cursor left(Cursor cursor, bool allowLineChange) {
    cursor = fix(cursor);
    if (cursor.x() == 0) {
        if (cursor.y() > 0 && allowLineChange) {
            cursor.y(cursor.y() - 1);
            cursor.x(cursor.buffer().lines().at(cursor.y()).size());
        }
    }
    else {
        cursor.x(cursor.x() - 1);
    }
    return cursor;
}

Cursor home(Cursor cursor) {
    cursor.x(0);
    return cursor;
}

Cursor end(Cursor cursor) {
    const auto &lines = cursor.buffer().lines();
    if (lines.empty()) {
        return {cursor.buffer()};
    }
    cursor = fix(cursor);
    cursor.x(lines.at(cursor.y()).size());
    return cursor;
}

Cursor insert(Utf8Char c, Cursor cur) {
    cur = fix(cur);
    cur.buffer().apply({"", {1, c}, cur});

    return right(cur);
}

Cursor erase(Cursor cursor) {
    if (cursor.x() == 0 && cursor.y() == 0) {
        return cursor;
    }

    auto end = fix(cursor);
    cursor = left(cursor);
    erase(CursorRange{cursor, end});

    return cursor;
}

Cursor deleteLine(Cursor cursor) {
    if (cursor.buffer().lines().empty()) {
        return {cursor.buffer()};
    }
    cursor = fix(cursor);

    if (cursor.buffer().lines().size() == cursor.y() + 1) {
        return erase(CursorRange{left(home(cursor)), end(cursor)});
    }

    return erase(CursorRange{home(cursor), right(end(cursor))});
}

Cursor join(Cursor cursor) {
    auto &buffer = cursor.buffer();
    {
        auto &lines = buffer.lines();

        if (lines.size() <= cursor.y() + 1) {
            return cursor;
        }
    }

    auto beginLine = Cursor{buffer, 0, cursor.y() + 1};

    return erase(beginLine);
}

Cursor split(Cursor cursor) {
    auto &buffer = cursor.buffer();
    auto &lines = cursor.buffer().lines();
    if (lines.empty()) {
        insert('\n', cursor.buffer().end());
    }

    if (cursor.y() >= lines.size()) {
        return fix(cursor);
    }
    else {
        const auto &line = lines.at(cursor.y());
        cursor.buffer().apply({"", "\n", cursor});
        return {cursor.buffer(), 0, cursor.y() + 1};
    }
}

FString indentation(Cursor cursor) {
    auto &line = lineAt(cursor);

    FString indentationStr;
    for (auto c : line) {
        if (c.c == ' ' || c.c == '\t') {
            indentationStr += std::string{c};
        }
        else {
            break;
        }
    }

    return indentationStr;
}

Cursor copyIndentation(Cursor cursor, std::string autoIndentString) {
    cursor = fix(cursor);

    if (cursor.y() == 0) {
        return cursor;
    }

    auto &lines = cursor.buffer().lines();

    auto &line = cursor.buffer().lineAt(cursor.y());
    auto &lineAbove = lines.at(cursor.y() - 1);
    auto indentationStr = indentation({cursor.buffer(), 0, cursor.y() - 1});

    cursor.x(indentationStr.size());

    if (!lineAbove.empty() && lineAbove.at(lineAbove.size() - 1).c == '{') {
        indentationStr += autoIndentString;
        cursor.x(cursor.x() + autoIndentString.size());
    }

    auto edit = BufferEdit{
        "",
        indentationStr,
        {cursor.buffer(), 0, cursor.y()},
    };

    apply(edit);
    return cursor;
}

Cursor wordBegin(Cursor cursor) {
    auto type = getType(cursor);

    while ((cursor.x() > 0 || cursor.y() > 0) &&
           type == InternalCharType::Space) {
        cursor = left(cursor);
        type = getType(cursor);
    }

    while (cursor.x() > 0 || cursor.y() > 0) {
        auto nCursor = left(cursor);
        auto nType = getType(nCursor);
        if (nType == type) {
            cursor = nCursor;
        }
        else {
            return cursor;
        }
    }
    return cursor;
}

Cursor nextWord(Cursor cursor, bool allowLineChange) {
    auto cur = cursor;
    auto &buffer = cur.buffer();
    auto startType = getType(cur);

    auto end = buffer.end();

    for (; cur < end; cur = right(cur)) {
        if (cur.y() != cursor.y()) {
            if (!allowLineChange) {
                return ::end(cursor);
            }
            else {
                // This is a really wierd one from vim
                if (cur.buffer().lineAt(cur.y()).empty()) {
                    return cur;
                }
            }
        }

        auto type = getType(cur);
        if (type == InternalCharType::Space) {
            startType = InternalCharType::Space;
            continue;
        }
        if (type != startType) {
            break;
        }
    }

    return cur;
}

Cursor wordEnd(Cursor cursor) {
    auto &buffer = cursor.buffer();

    auto end = buffer.end();
    auto type = getType(cursor);

    while (cursor < end && type == InternalCharType::Space) {
        cursor = right(cursor);
        type = getType(cursor);
    }

    while (cursor < end) {
        auto nCursor = right(cursor);
        auto nType = getType(nCursor);
        if (nType == type) {
            cursor = nCursor;
        }
        else {
            return cursor;
        }
    }
    return cursor;
}

Utf8Char content(Cursor cursor) {
    cursor = fix(cursor);

    auto &line = cursor.buffer().lineAt(cursor.y());

    if (cursor.x() == line.size()) {
        return '\n';
    }

    return line.at(cursor.x()).c;
}

Cursor autocompleteWordBegin(const Cursor cursor) {
    auto currentChar = content(left(cursor)).at(0);
    Cursor begin = cursor;
    if (isalnum(currentChar)) {
        // If on for example a newline
        begin = wordBegin(cursor);
    }
    else {
        begin = cursor; // I.e. Empty string
    }

    return begin;
}

std::optional<Cursor> find(Cursor cursor,
                           Utf8Char c,
                           bool allowLineChange,
                           bool cursorBefore) {
    auto end = allowLineChange ? cursor.buffer().end() : ::end(cursor);
    auto prev = cursor;
    for (auto cur = cursor; cur < end; cur = right(cur, allowLineChange)) {
        if (content(cur) == c) {
            if (cursorBefore) {
                return prev;
            }
            return cur;
        }
        prev = cur;
    }
    return {};
}

std::optional<Cursor> rfind(Cursor cursor, Utf8Char c, bool allowLineChange) {
    for (auto cur = cursor;; cur = left(cur, allowLineChange)) {
        if (content(cur) == c) {
            return cur;
        }
        if (cur.x() == 0) {
            if (allowLineChange) {
                if (cur.y() == 0) {
                    return {};
                }
            }
            else {
                return {};
            }
        }
    }

    return {};
}

Cursor apply(BufferEdit edit) {
    return edit.position.buffer().apply(std::move(edit));
}

Cursor insert(Cursor cur, FString str) {
    cur = fix(cur);

    return apply(BufferEdit{"", std::move(str), cur});
}

Cursor up(Cursor cur) {
    if (cur.y() == 0) {
        return cur;
    }
    cur.y(cur.y() - 1);
    return cur;
}

Cursor down(Cursor cur) {
    cur.y(cur.y() + 1);
    return cur;
}

std::optional<Cursor> matchingLeft(Cursor cursor,
                                   const Utf8Char start,
                                   const Utf8Char stop,
                                   bool shouldEnableNestCheck) {
    if (start == stop) {
        shouldEnableNestCheck = false;
    }

    auto begin = cursor;
    int count = 1;

    for (;; begin = left(begin, true)) {
        auto c = content(begin);
        if (c == start) {
            --count;
        }
        // This is to handle nested stuff like { {} {} }
        if (shouldEnableNestCheck && c == stop) {
            ++count;
        }

        if (!count) {
            break;
        }
        if (begin.x() == 0 && begin.y() == 0) {
            return std::nullopt;
        }
    }
    if (count) {
        // Failed to find match
        return std::nullopt;
    }
    return begin;
}

std::optional<Cursor> matchingRight(Cursor cursor,
                                    const Utf8Char start,
                                    const Utf8Char stop,
                                    bool shouldEnableNestCheck) {
    if (start == stop) {
        shouldEnableNestCheck = false;
    }

    const auto bufferEnd = cursor.buffer().end();
    auto end = cursor;
    int count = 1;
    for (; end != bufferEnd; end = right(end, true)) {
        if (content(end) == stop) {
            --count;
        }
        if (shouldEnableNestCheck && content(end) == start) {
            ++count;
        }
        if (!count) {
            break;
        }
    }
    if (count) {
        // Failed to find match
        return std::nullopt;
    }
    return end;
}

/// The bool is if the match is forward else backward
std::pair<char, bool> getMatchType(Utf8Char c) {
    switch (static_cast<int>(c)) {
    case '(':
        return {')', true};
    case ')':
        return {'(', false};
    case '{':
        return {'}', true};
    case '}':
        return {'{', false};
    case '[':
        return {']', true};
    case ']':
        return {'[', false};
    }

    return {0, false};
}

// Cursor fixBraceIndentation(Cursor cursor) {}

Cursor findMatching(Cursor cursor) {
    auto startC = content(cursor);
    auto [c, forward] = getMatchType(startC);

    if (forward) {
        cursor = right(cursor);
        auto res = matchingRight(cursor, startC, c, true);
        return res ? *res : cursor;
    }
    else {
        cursor = left(cursor);
        auto res = matchingLeft(cursor, c, startC, true);
        return res ? *res : cursor;
    }

    return cursor;
}

const FString &lineAt(Cursor cursor) {
    return cursor.buffer().lineAt(cursor.y());
}

Cursor lastNonSpaceOnLine(Cursor cursor) {
    for (auto cur = cursor; cur != end(cursor); ++cur) {
        auto c = content(cur);
        if (!isSpace(c)) {
            cursor = cur;
        }
    }

    return cursor;
}

Cursor firstNonSpaceOnLine(Cursor cursor) {
    for (auto cur = home(cursor); cur != end(cursor); ++cur) {
        auto c = content(cur);
        cursor = cur;
        if (!isSpace(c)) {
            return cursor;
        }
    }

    return cursor;
}
