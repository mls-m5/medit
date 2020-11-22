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

    auto text = buffer.text();
    if (_history.empty() || text != _history.back().old) {
        // Only push changes if there is actually a change
        _history.emplace_back(Item{std::move(text)});
        _currentState.copyFrom(buffer);

        if (_ignoreRedoClear) {
            // Ignore one commit after a redo is done
            _ignoreRedoClear = false;
        }
        else {
            _redo.clear();
        }
    }
}

void History::undo(Buffer &buffer) {
    if (_history.empty()) {
        return;
    }

    auto text = buffer.text();
    auto comparison = text;
    if (_history.back().old == comparison) {
        _redo.push_back(Item{std::move(text)});
        _history.pop_back();
        if (_history.empty()) {
            return;
        }
    }

    buffer.text(_history.back().old);
}

void History::redo(Buffer &buffer) {
    if (_redo.empty()) {
        return;
    }

    auto text = buffer.text();

    _ignoreRedoClear = true;

    if (text == _redo.back().old) {
        _redo.pop_back();
    }

    if (!_redo.empty()) {
        buffer.text(_redo.back().old);
    }
}

void History::clear() {
    _history.clear();
}
