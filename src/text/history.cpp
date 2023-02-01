#include "text/history.h"
#include "bufferedit.h"
#include "cursorrangeops.h"
#include "text/buffer.h"

void History::commit() {
    // TODO: Only commit the part that change
    auto text = _buffer.ftext();

    auto edit = BufferEdit{
        .from = _currentState,
        .to = text,
        .position = _buffer.cursor({0, 0}),
    };

    edit.trim();

    if (edit.empty()) {
        return;
    }

    if (text != _currentState) {
        _history.push_back({
            .edit = std::move(edit),
            .revision = ++_revision,
        });
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

    auto edit = revert(_history.back().edit);

    _buffer.apply(edit);

    _redo.push_back(std::move(_history.back()));
    _history.pop_back();

    ++_revision;
}

void History::redo() {
    if (_redo.empty()) {
        return;
    }

    auto text = _buffer.text();
    _ignoreRedoClear = true;
    _buffer.apply(_redo.back().edit);

    ++_revision;
}

void History::clear() {
    _history.clear();
    _redo.clear();
}
