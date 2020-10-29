
#include "cursor.h"
#include "buffer.h"

Cursor fix(Cursor cursor) {
    auto &buffer = cursor.buffer();
    auto &lines = cursor.buffer().lines();
    if (buffer.lines().empty()) {
        cursor = {buffer, 0, 0};
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
    cursor = fix(cursor);
    return cursor.x(cursor.buffer().lines().at(cursor.y()).size());
}
