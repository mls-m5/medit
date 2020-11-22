#include "text/history.h"
#include "text/buffer.h"

namespace {
// History::Item getDiff(const Buffer &oldBuffer, const Buffer &newBuffer) {
//    return {};
//}
} // namespace

void History::commit(const Buffer &buffer) {
    //    auto diff = getDiff(_currentState, buffer);

    // Todo: This is a dumb implementation, only commit changed part of file in
    // the future

#warning "base this check on changed time"
    auto text = buffer.text();
    if (!(!_history.empty() && text != _history.back().old)) {
        // Only push changes if there is actually a change
        _history.emplace_back(Item{std::move(text)});
    }

    _currentState.copyFrom(buffer);
}

void History::undo(Buffer &buffer) {
    // Todo: Improve in the future

    if (_history.empty()) {
        return;
    }

    auto comparison = buffer.text();
    if (_history.back().old == comparison) {
        _history.pop_back();
    }

    buffer.text(_history.back().old);
}

void History::clear() {
    _history.clear();
}
