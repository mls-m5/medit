#pragma once

#include "meditfwd.h"
#include "text/cursor.h"

class EditorCursor {
private:
    Position _position;
    Editor *_editor = nullptr;

public:
    EditorCursor(const EditorCursor &) = default;
    EditorCursor(EditorCursor &&) = default;
    EditorCursor &operator=(const EditorCursor &) = default;
    EditorCursor &operator=(EditorCursor &&) = default;
    ~EditorCursor() = default;

    /// Get the cursor from the editor
    EditorCursor(Editor &);

    /// Get a cursor position at some specified position
    EditorCursor(Editor &, Position);
    EditorCursor(Editor &, Cursor);

    Cursor cursor() const;

    Position position() const;
    void position(const Position &);
    Editor &editor() const;
    void editor(Editor *);

    operator Cursor() {
        return cursor();
    }

    EditorCursor fix() const;
};
