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

void History::event(HistoryKind kind) {
    _history.push_back({
        .edit = BufferEdit{{}, {}, Cursor{*_buffer}},
        .revision = _revision,
        .kind = kind,
    });
}

void History::tagSince(long revision, HistoryKind kind) {
    for (auto it = _history.rbegin();
         it != _history.rend() && it->revision > revision;
         ++it) {
        if (!it->isMarker()) {
            it->kind = kind;
        }
    }
}

History::Item *History::lastEdit() {
    for (auto it = _history.rbegin(); it != _history.rend(); ++it) {
        if (!it->isMarker()) {
            return &*it;
        }
    }
    return nullptr;
}

void History::markMajor() {
    if (auto *item = lastEdit()) {
        item->isMajor = true;
    }
}

void History::undoMajor() {
    undo();

    for (auto *item = lastEdit(); item && !item->isMajor; item = lastEdit()) {
        undo();
    }
}

void History::undo() {
    if (!lastEdit()) {
        return;
    }

    // Markers after the edit go with it, so redo puts them back after it.
    while (_history.back().isMarker()) {
        _redo.push_back(std::move(_history.back()));
        _history.pop_back();
    }

    auto edit = revert(_history.back().edit);

    _buffer->applyWithoutHistory(edit);

    _redo.push_back(std::move(_history.back()));
    _history.pop_back();

    ++_revision;
}

void History::redo() {
    if (_redo.empty()) {
        return;
    }

    _buffer->applyWithoutHistory(_redo.back().edit);

    _history.push_back(std::move(_redo.back()));
    _redo.pop_back();

    // The markers that were undone with this edit.
    while (!_redo.empty() && _redo.back().isMarker()) {
        _history.push_back(std::move(_redo.back()));
        _redo.pop_back();
    }

    ++_revision;
}

void History::clear() {
    _history.clear();
    _redo.clear();
}
