#include "editorcursor.h"
#include "views/editor.h"

Position EditorCursor::position() const {
    return _position;
}

void EditorCursor::position(const Position &newPosition) {
    _position = newPosition;
}

Editor &EditorCursor::editor() const {
    return *_editor;
}

void EditorCursor::editor(Editor *newEditor) {
    _editor = newEditor;
}

EditorCursor EditorCursor::fix() const {
    return {*_editor, _editor->fix(cursor())};
}

EditorCursor::EditorCursor(Editor &editor, Position position)
    : _editor{&editor}
    , _position{position} {}

EditorCursor::EditorCursor(Editor &editor)
    : _editor{&editor}
    , _position{editor.cursor().pos()} {}

EditorCursor::EditorCursor(Editor &editor, Cursor cursor)
    : _editor{&editor}
    , _position{cursor.pos()} {}

Cursor EditorCursor::cursor() const {
    return {_editor->buffer(), _position.x(), _position.y()};
}
