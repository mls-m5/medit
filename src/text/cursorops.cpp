
#include "cursorops.h"
#include "buffer.h"
#include "cursorrangeops.h"
#include "syntax/basichighligting.h"
#include "views/bufferview.h"

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

    auto line = lines.at(cursor.y());
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
    auto line = lines.at(cursor.y());

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

Cursor copyIndentation(Cursor cursor, std::string autoIndentString) {
    cursor = fix(cursor);

    if (cursor.y() == 0) {
        return cursor;
    }

    auto &lines = cursor.buffer().lines();

    auto &line = cursor.buffer().lineAt(cursor.y());
    auto &lineAbove = lines.at(cursor.y() - 1);

    std::string indentationStr;
    for (auto c : lineAbove) {
        if (c.c == ' ' || c.c == '\t') {
            indentationStr += std::string{c};
        }
        else {
            break;
        }
    }

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
    // space = 0, alnum = 1, other = 2
    auto getType = [](Cursor cursor) {
        auto c = content(cursor).front();
        return (isalnum(c) || c == '_') ? 1 : ((!isspace(c)) * 2);
    };
    auto type = getType(cursor);

    while ((cursor.x() > 0 || cursor.y() > 0) && type == 0) {
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

Cursor wordEnd(Cursor cursor) {
    // space = 0, alnum = 1, other = 2
    auto getType = [](Cursor cursor) {
        auto c = content(cursor).front();
        return (isalnum(c) || c == '_') ? 1 : ((!isspace(c)) * 2);
    };

    auto &buffer = cursor.buffer();

    auto end = buffer.end();
    auto type = getType(cursor);

    while (cursor < end && type == 0) {
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

std::optional<Cursor> find(Cursor cursor, Utf8Char c) {
    auto end = ::end(cursor);
    for (auto cur = cursor; cur != end; cur = right(cur, false)) {
        if (content(cur) == c) {
            return cur;
        }
    }
    return {};
}

std::optional<Cursor> rfind(Cursor cursor, Utf8Char c) {
    for (auto cur = cursor;; cur = left(cur, false)) {
        if (content(cur) == c) {
            return cur;
        }
        if (cur.x() == 0) {
            return {};
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
    return fix(cur);
}
