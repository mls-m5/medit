#include "text/history.h"
#include "text/buffer.h"

void History::commit() {
    //    auto diff = getDiff(_currentState, buffer);

    // Todo: This is a dumb implementation, only commit changed part of file in
    // the future

    auto text = _buffer.text();
    if (_history.empty() || text != _history.back().old) {
        // Only push changes if there is actually a change
        _history.emplace_back(Item{std::move(text)});
        _currentState = _buffer.text();

        if (_ignoreRedoClear) {
            // Ignore one commit after a redo is done
            _ignoreRedoClear = false;
        }
        else {
            _redo.clear();
        }
    }
}

void History::undo() {
    if (_history.empty()) {
        return;
    }

    auto text = _buffer.text();
    auto comparison = text;
    if (_history.back().old == comparison) {
        _redo.push_back(Item{std::move(text)});
        _history.pop_back();
        if (_history.empty()) {
            return;
        }
    }

    _buffer.text(_history.back().old);
}

void History::redo() {
    if (_redo.empty()) {
        return;
    }

    auto text = _buffer.text();

    _ignoreRedoClear = true;

    if (text == _redo.back().old) {
        _redo.pop_back();
    }

    if (!_redo.empty()) {
        _buffer.text(_redo.back().old);
    }
}

void History::clear() {
    _history.clear();
}
