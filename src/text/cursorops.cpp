#include "cursorops.h"

#include "buffer.h"
#include "colorize.h"

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
        cursor.y() = lines.size() - 1;
    }

    auto line = lines.at(cursor.y());
    if (line.empty()) {
        cursor.x() = 0;
    }
    else if (cursor.x() > line.size()) {
        cursor.x() = line.size();
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
            cursor.x() = 0;
            cursor.y() += 1;
        }
    }
    else {
        ++cursor.x();
    }

    return cursor;
}

Cursor left(Cursor cursor, bool allowLineChange) {
    cursor = fix(cursor);
    if (cursor.x() == 0) {
        if (cursor.y() > 0 && allowLineChange) {
            cursor.y() -= 1;
            cursor.x() = cursor.buffer().lines().at(cursor.y()).size();
        }
    }
    else {
        cursor.x() -= 1;
    }
    return cursor;
}

Cursor home(Cursor cursor) {
    return cursor.x(0);
}

Cursor end(Cursor cursor) {
    const auto &lines = cursor.buffer().lines();
    if (lines.empty()) {
        return {cursor.buffer()};
    }
    cursor = fix(cursor);
    return cursor.x(lines.at(cursor.y()).size());
}

Cursor insert(Utf8Char c, Cursor cur) {
    auto &lines = cur.buffer().lines();
    if (lines.empty()) {
        lines.emplace_back();
    }
    cur = fix(cur);
    if (c == '\n') {
        return split(cur);
    }
    else {
        auto &line = lines.at(cur.y());
        line.insert(cur.x(), c);
        colorize(line);
        cur.x() += 1;
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
    auto &line = lines.at(cursor.y());
    if (cursor.x() == 0) {
        if (cursor.y() > 0) {
            cursor.y() -= 1;
            return join(cursor);
        }
    }
    else {
        line.erase(cursor.x() - 1, 1);
        colorize(line);
        cursor.x() -= 1;
    }
    return cursor;
}

Cursor deleteLine(Cursor cursor) {
    auto &lines = cursor.buffer().lines();
    if (cursor.buffer().lines().empty()) {
        return {cursor.buffer()};
    }
    cursor = fix(cursor);
    lines.erase(lines.begin() + cursor.y());
    return cursor;
}

Cursor join(Cursor cursor) {
    auto &lines = cursor.buffer().lines();

    cursor = end(cursor);

    if (lines.size() <= cursor.y() + 1) {
        return cursor;
    }

    auto &line1 = lines.at(cursor.y());
    auto &line2 = lines.at(cursor.y() + 1);
    auto x = line1.size();

    line1 += line2;
    deleteLine({cursor.buffer(), 0, cursor.y() + 1});
    colorize(line1);

    return {cursor.buffer(), x, cursor.y()};
}

Cursor split(Cursor cursor) {
    auto &lines = cursor.buffer().lines();
    if (lines.empty()) {
        lines.emplace_back();
    }

    if (cursor.y() >= lines.size()) {
        return fix(cursor);
    }
    else {
        const auto &line = lines.at(cursor.y());

        if (cursor.x() >= line.size()) {
            lines.insert(lines.begin() + cursor.y() + 1, FString{});
        }
        else {
            lines.insert(lines.begin() + cursor.y() + 1,
                         {
                             line.begin() + cursor.x(),
                             line.end(),
                         });

            auto &l = lines.at(cursor.y());

            l.erase(l.begin() + cursor.x(), l.end());
        }
        return {cursor.buffer(), 0, cursor.y() + 1};
    }

    return cursor;
}