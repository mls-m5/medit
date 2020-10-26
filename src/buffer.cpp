#include "buffer.h"
#include <sstream>

Cursor Buffer::fixCursor(Cursor cursor) {
    if (_lines.empty()) {
        cursor = {0, 0};
    }
    else if (cursor.y >= _lines.size()) {
        cursor.y = _lines.size() - 1;
    }

    auto line = _lines.at(cursor.y);
    if (line.empty()) {
        cursor.x = 0;
    }
    else if (cursor.x > line.size()) {
        cursor.x = line.size();
    }
    return cursor;
}

Cursor Buffer::prev(Cursor cursor) {
    cursor = fixCursor(cursor);
    if (cursor.x == 0) {
        if (cursor.y > 0) {
            cursor.y -= 1;
            cursor.x = _lines.at(cursor.y).size();
        }
    }
    else {
        cursor.x -= 1;
    }
    return cursor;
}

Cursor Buffer::next(Cursor cursor) {
    if (_lines.empty()) {
        return {};
    }

    cursor = fixCursor(cursor);
    auto line = _lines.at(cursor.y);

    if (cursor.x == line.size()) {
        if (cursor.y == _lines.size() - 1) {
            return cursor;
        }
        cursor.x = 0;
        cursor.y += 1;
    }
    else {
        ++cursor.x;
    }

    return cursor;
}

Cursor Buffer::insert(Utf8Char c, Cursor cur) {
    if (_lines.empty()) {
        _lines.emplace_back();
    }
    cur = fixCursor(cur);
    if (c == '\n') {
        _lines.insert(_lines.begin() + cur.y + 1, FString{});
        cur.y += 1;
        cur.x = 0;
    }
    else {
        auto &line = _lines.at(cur.y);
        line.insert(cur.x, c);
        cur.x += 1;
    }

    return cur;
}

Cursor Buffer::erase(Cursor cur) {
    if (_lines.empty()) {
        return {};
    }
    else if (_lines.size() == 1 && _lines.front().empty()) {
        return {};
    }
    cur = fixCursor(cur);
    auto &line = _lines.at(cur.y);
    if (cur.x == 0 && cur.y > 0) {
        auto oldLine = std::move(_lines.at(cur.y));
        _lines.erase(_lines.begin() + cur.y);
        cur.y -= 1;
        auto &lineAbove = _lines.at(cur.y);
        cur.x = lineAbove.size();
        lineAbove += oldLine;
    }
    else {
        line.erase(cur.x - 1, 1);
        cur.x -= 1;
    }
    return cur;
}

std::string Buffer::text() {
    std::ostringstream ss;
    for (auto &line : _lines) {
        ss << std::string{line} << "\n";
    }
    auto ret = ss.str();
    ret.pop_back(); // Remove extra newline
    return ret;
}

void Buffer::text(std::string text) {
    std::istringstream ss{move(text)};

    for (std::string line; getline(ss, line);) {
        _lines.emplace_back(std::move(line));
    }
}
