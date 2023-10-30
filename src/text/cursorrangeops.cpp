#include "text/cursorrangeops.h"
#include "bufferedit.h"
#include "text/buffer.h"
#include "text/cursorops.h"
#include "text/cursorrange.h"
#include <iostream>
#include <sstream>

//! Returns a list with at least one element
std::vector<FString> contentLines(CursorRange range) {
    auto begin = fix(range.begin());
    auto end = fix(range.end());

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

FString content(CursorRange range) {
    return FString::join(contentLines(range));
}

void format(CursorRange range, FormatType format) {
    range.buffer().format(range, format);
}

Cursor erase(CursorRange range) {
    range.beginPosition(fix(range.begin()));
    range.endPosition(fix(range.end()));

    auto oldText = content(range);

    BufferEdit edit = {
        .from = content(range),
        .to = "",
        .position = range.begin(),
    };

    range.buffer().apply(std::move(edit));

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
        if (c.c.at(0) != str.at(i)) {
            return false;
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
    stream << content(range);
    return stream;
}

std::string toString(CursorRange range) {
    return content(range);
}

CursorRange fix(CursorRange range) {
    return {fix(range.begin()), fix(range.end())};
}

CursorRange word(Cursor cursor) {
    return {wordBegin(cursor), ++wordEnd(cursor)};
}

CursorRange line(Cursor cursor) {
    return {home(cursor), end(cursor)};
}

CursorRange inner(const Cursor cursor,
                  const Utf8Char start,
                  const Utf8Char stop) {
    auto begin = cursor;
    for (; content(begin) != start; begin = left(begin, true)) {
        if (begin.x() == 0 && begin.y() == 0) {
            return {cursor};
        }
    }

    const auto bufferEnd = cursor.buffer().end();
    auto end = cursor;
    for (; content(end) != stop; end = right(end, true)) {
    }

    if (end == bufferEnd) {
        return {cursor};
    }

    return {++begin, end};
}

CursorRange all(Buffer &buffer) {
    return {buffer.begin(), buffer.end()};
}

Cursor replace(CursorRange range, FString str) {
    return apply(BufferEdit{
        content(range),
        std::move(str),
        range.begin(),
    });
}

CursorRange extendRight(CursorRange range, size_t num, bool allowLineChanges) {
    auto r = range.end();
    for (size_t i = 0; i < num; ++i) {
        r = right(r, allowLineChanges);
    }
    return CursorRange{range.begin(), r};
}

CursorRange extendLeft(CursorRange range, size_t num, bool allowLineChanges) {
    auto l = range.begin();
    for (size_t i = 0; i < num; ++i) {
        l = left(l, allowLineChanges);
    }
    return CursorRange{l, range.end()};
}

CursorRange extend(CursorRange range, size_t num, bool allowLineChanges) {
    return extendRight(
        extendLeft(range, num, allowLineChanges), num, allowLineChanges);
}
