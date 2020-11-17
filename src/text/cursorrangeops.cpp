

#include "text/cursorrangeops.h"
#include "text/buffer.h"
#include "text/cursorops.h"
#include <iostream>

//! Returns a list with at least one element
std::vector<FString> content(CursorRange range) {
    auto begin = fix(range.begin());
    auto end = fix(range.end());

    auto &buffer = begin.buffer();

    if (begin.y() == end.y()) {
        auto line = buffer.lineAtConst(begin.y());
        return {{line.begin() + begin.x(), line.begin() + end.x()}};
    }

    auto res = std::vector<FString>{buffer.lines().begin() + begin.y(),
                                    buffer.lines().begin() + end.y() + 1};

    res.front().erase(0, begin.x());

    res.back().erase(res.back().begin() + end.x(), res.back().end());

    return res;
}

void format(CursorRange range, FormatType format) {
    for (auto it : range) {
        if (it) {
            it->f = format;
        }
    }
}

Cursor erase(CursorRange range) {
    auto begin = fix(range.begin());
    auto end = fix(range.end());

    if (begin == end) {
        return range.begin();
    }

    auto &buffer = begin.buffer();

    if (begin.y() == end.y()) {
        auto &line = buffer.lineAt(begin.y());
        line.erase(line.begin() + begin.x(), line.begin() + end.x());
        return begin;
    }

    {
        auto &line = buffer.lineAt(begin.y());
        line.erase(line.begin() + begin.x(), line.end());

        auto &backLine = buffer.lineAt(end.y());
        line.insert(line.end(), backLine.begin() + end.x(), backLine.end());
    }

    buffer.deleteLine(begin.y() + 1, end.y() - begin.y());

    return range.begin();
}

bool operator==(CursorRange range, std::string_view str) {
    // Only for non utf-8 chars for now

    if (str.empty()) {
        if (range.empty()) {
            return true;
        }
        else {
            return false;
        }
    }

    size_t i = 0;

    for (auto c : range) {
        if (i >= str.size()) {
            return false;
            break;
        }
        if (c == nullptr) {
            if (str.at(i) != '\n') {
                return false;
            }
        }
        else {
            if (c->c.at(0) != str.at(i)) {
                return false;
            }
        }
        ++i;
    }

    if (i == str.size()) {
        return true;
    }
    else {
        return false;
    }
}

std::ostream &operator<<(std::ostream &stream, CursorRange range) {
    for (auto c : range) {
        if (c) {
            stream.write(&c->c.front(), c->c.size());
        }
        else {
            stream << "\n";
        }
    }
    return stream;
}
