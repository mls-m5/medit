

#include "text/cursorrangeops.h"
#include "text/buffer.h"
#include "text/cursorops.h"

//! Returns a list with at least one element
std::vector<FString> content(CursorRange range) {
    auto begin = fix(range.beginCursor());
    auto end = fix(range.endCursor());

    auto &buffer = begin.buffer();

    if (begin.y() == end.y()) {
        auto line = buffer.lineAt(begin.y());
        return {{line.begin() + begin.x(), line.begin() + end.x()}};
    }

    auto res = std::vector<FString>{buffer.lines().begin() + begin.y(),
                                    buffer.lines().begin() + end.y() + 1};

    res.front().erase(0, begin.x());

    res.back().erase(res.back().begin() + end.x(), res.back().end());

    return res;
}

void format(CursorRange range, FormatType format) {
    for (auto it = range.beginCursor(); it < range.endCursor();
         it = right(it)) {
        contentPtr(it)->f = format;
    }
}
