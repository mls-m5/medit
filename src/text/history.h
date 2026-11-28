#pragma once

#include "text/bufferedit.h"
#include <vector>

class Buffer;

/// What a history item records. Every change to a buffer goes through the
/// history, also those that do not come from typing, so that the history can
/// later be shared, branched and rebased.
enum class HistoryKind {
    Edit,   // Typed or made by a command
    Reload, // Taken from the file on disk, replacing the buffer's text
    Merge,  // Taken from the file on disk, merged with unsaved changes
    Load,   // Marker: the buffer was loaded from its file
    Save,   // Marker: the buffer was saved to its file
};

class History {
public:
    History(Buffer &buffer)
        : _buffer{&buffer} {}

    struct Item {
        BufferEdit edit;
        long revision = {};

        bool isMajor = false;

        HistoryKind kind = HistoryKind::Edit;

        /// Markers record an event without changing the text; undo and redo
        /// pass over them.
        bool isMarker() const {
            return kind == HistoryKind::Load || kind == HistoryKind::Save;
        }

        void trim();
    };

    //! Add the last change to the history
    //! Replace this with a function that only takes the specific changes
    bool commit(BufferEdit edit);

    /// Records a marker (Load or Save) at the current state.
    void event(HistoryKind kind);

    /// Sets the kind of the edits committed after `revision`.
    void tagSince(long revision, HistoryKind kind);

    //! Remember that the last change was a change you might want to go back to
    void markMajor();

    void undoMajor();

    void undo();
    void redo();

    //! If you do not want changes to go back any longer
    //! example for when loading a file
    void clear();

    long revision() {
        return _revision;
    }

    /// Oldest first.
    const std::vector<Item> &items() const {
        return _history;
    }

private:
    /// The last item that is not a marker, or null.
    Item *lastEdit();

    long _revision = 1;

    Buffer *_buffer = nullptr;

    std::vector<Item> _history;
    std::vector<Item> _redo;

    friend RawBuffer;
};
