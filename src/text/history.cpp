#include "text/history.h"
#include "bufferedit.h"
#include "text/buffer.h"

bool History::commit(BufferEdit edit) {
    edit.trim();

    if (edit.empty()) {
        return false;
    }

    _history.push_back({
        .edit = std::move(edit),
        .revision = ++_revision,
    });

    _redo.clear();

    return true;
}

void History::markMajor() {
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

    ++_revision;
}

void History::clear() {
    _history.clear();
    _redo.clear();
    //    _currentState = _buffer.text();
}
