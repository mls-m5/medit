#include "text/history.h"
#include "bufferedit.h"
#include "text/buffer.h"

bool History::commit() {
    // TODO: Only commit the part that change
    auto text = _buffer.ftext();

    auto edit = BufferEdit{
        .from = _currentState,
        .to = text,
        .position = _buffer.cursor({0, 0}),
    };

    edit.trim();

    if (edit.empty()) {
        return false;
    }

    if (text != _currentState) {
        _history.push_back({
            .edit = std::move(edit),
            .revision = ++_revision,
        });
        _currentState = _buffer.text();

        _redo.clear();
    }

    return true;
}

void History::markMajor() {
    if (_history.empty()) {
        commit();
    }
    if (_history.empty()) {
        return;
    }
    _history.back().isMajor = true;
}

void History::undoMajor() {
    undo();

    while (!_history.empty() && !_history.back().isMajor) {
        undo();
    }
}

void History::undo() {
    if (_history.empty()) {
        return;
    }

    auto edit = revert(_history.back().edit);

    _buffer.applyWithoutHistory(edit);

    _redo.push_back(std::move(_history.back()));
    _history.pop_back();

    _currentState = _buffer.text();

    ++_revision;
}

void History::redo() {
    if (_redo.empty()) {
        return;
    }

    auto text = _buffer.text();
    //    _ignoreRedoClear = true;
    _buffer.applyWithoutHistory(_redo.back().edit);

    _history.push_back(std::move(_redo.back()));

    _redo.pop_back();

    _currentState = _buffer.text();

    ++_revision;
}

void History::clear() {
    _history.clear();
    _redo.clear();
    _currentState = _buffer.text();
}
