#pragma once

#include "cursorops.h"
#include "cursorrange.h"
#include <functional>

/// Helper class to split up a range into words
class Words {
public:
    static auto next(Cursor cursor) {
        cursor = wordEnd(cursor);
        if (isspace(content(cursor).at(0))) {
            return CursorRange{cursor, cursor};
        }
        auto begin = wordBegin(cursor);
        auto end = right(cursor);
        return CursorRange{begin, end};
    }

    struct Iterator {
        CursorRange *fullRange;
        CursorRange range;

        Iterator(CursorRange *fullRange, CursorRange range)
            : fullRange(fullRange)
            , range(range) {}

        Iterator &operator++() {
            auto cursor = range.end();
            if (cursor == fullRange->end()) {
                range = {cursor};
            }
            else {
                range = next(cursor);
            }

            return *this;
        };

        CursorRange operator*() {
            return range;
        }

        //! Just check for end
        bool operator!=(const Iterator &it) {
            return !(it.range.empty() && range.empty());
        }
    };

    using NextFuncT = std::function<void(Iterator &)>;

    Iterator begin() {
        return Iterator{&_range, next(_range.begin())};
    }

    Iterator end() {
        // Empty range
        return Iterator{
            &_range, CursorRange{_range.buffer(), _range.end(), _range.end()}};
    }

    Words(CursorRange range)
        : _range(range) {}

private:
    CursorRange _range;
};
