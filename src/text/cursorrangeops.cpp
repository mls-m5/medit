

#include "text/cursorrangeops.h"
#include "text/buffer.h"
#include "text/cursorops.h"

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
    //    for (auto it = range.begin(); it < range.end();
    //         it = right(it)) {
    for (auto it : range) {
        if (it) {
            it->f = format;
        }
        //        contentPtr(it)->f = format;
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
