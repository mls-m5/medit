
#include "cursorops.h"
#include "buffer.h"
#include "colorize.h"
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
    auto &lines = cur.buffer().lines();
    if (lines.empty()) {
        cur.buffer().push_back();
    }
    cur = fix(cur);
    if (c == '\n') {
        return split(cur);
    }
    else {
        auto &line = cur.buffer().lineAt(cur.y());
        line.insert(cur.x(), c);
        colorize(line);
        cur.x(cur.x() + 1);
    }

    return cur;
}

Cursor erase(Cursor cursor) {
    auto &lines = cursor.buffer().lines();
    if (lines.empty()) {
        return {cursor.buffer()};
    }
    else if (lines.size() == 1 && lines.front().empty()) {
        return {cursor.buffer()};
    }
    cursor = fix(cursor);
    auto &line = cursor.buffer().lineAt(cursor.y());
    if (cursor.x() == 0) {
        if (cursor.y() > 0) {
            cursor.y(cursor.y() - 1);
            return join(cursor);
        }
    }
    else {
        line.erase(cursor.x() - 1, 1);
        colorize(line);
        cursor.x(cursor.x() - 1);
    }
    return cursor;
}

Cursor deleteLine(Cursor cursor) {
    if (cursor.buffer().lines().empty()) {
        return {cursor.buffer()};
    }
    cursor = fix(cursor);
    cursor.buffer().deleteLine(cursor.y());
    return cursor;
}

Cursor join(Cursor cursor) {
    auto &buffer = cursor.buffer();
    auto &lines = buffer.lines();

    cursor = end(cursor);

    if (lines.size() <= cursor.y() + 1) {
        return cursor;
    }

    auto &line1 = buffer.lineAt(cursor.y());
    auto &line2 = buffer.lineAt(cursor.y() + 1);
    auto x = line1.size();

    line1 += line2;
    deleteLine({cursor.buffer(), 0, cursor.y() + 1});
    colorize(line1);

    return {cursor.buffer(), x, cursor.y()};
}

Cursor split(Cursor cursor) {
    auto &buffer = cursor.buffer();
    auto &lines = cursor.buffer().lines();
    if (lines.empty()) {
        cursor.buffer().push_back();
    }

    if (cursor.y() >= lines.size()) {
        return fix(cursor);
    }
    else {
        const auto &line = lines.at(cursor.y());

        if (cursor.x() >= line.size()) {
            cursor.buffer().insert(cursor.y() + 1, FString{});
        }
        else {
            cursor.buffer().insert(cursor.y() + 1,
                                   {
                                       line.begin() + cursor.x(),
                                       line.end(),
                                   });

            auto &l = buffer.lineAt(cursor.y());

            l.erase(l.begin() + cursor.x(), l.end());
        }
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

    while (!line.empty() && isspace(line.at(0).c.at(0))) {
        line.erase(line.begin(), line.begin() + 1);
    }

    cursor.x(indentationStr.size());

    if (!lineAbove.empty() && lineAbove.at(lineAbove.size() - 1).c == '{') {
        indentationStr += autoIndentString;
        cursor.x(cursor.x() + autoIndentString.size());
    }

    line.insert(line.begin(), FString{indentationStr});
    return cursor;
}

Cursor wordBegin(Cursor cursor) {
    // space = 0, alnum = 1, other = 2
    auto getType = [](Cursor cursor) {
        auto c = content(cursor).front();
        return isalnum(c) ? 1 : ((!isspace(c)) * 2);
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
        return isalnum(c) ? 1 : ((!isspace(c)) * 2);
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

FChar *contentPtr(Cursor cursor) {
    cursor = fix(cursor);

    auto &line = cursor.buffer().lineAt(cursor.y());

    if (cursor.x() == line.size()) {
        return nullptr;
    }

    return &line.at(cursor.x());
}
