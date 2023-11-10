#include "bufferedit.h"

void BufferEdit::trim() {
    for (; !to.empty() && !from.empty();) {
        if (to.back() == from.back()) {
            to.pop_back();
            from.pop_back();
            continue;
        }
        break;
    }

    auto minLen = std::min(to.size(), from.size());

    size_t erasedChars = 0;

    for (size_t i = 0; i < minLen; ++i) {
        if (to.at(i) == from.at(i)) {
            position += to.at(i).c.front();
            //            ++position;
            ++erasedChars;
            continue;
        }
        break;
    }

    to.erase(0, erasedChars);
    from.erase(0, erasedChars);
}

BufferEdit revert(BufferEdit e) {
    std::swap(e.from, e.to);
    return e;
}
