#include "views/editor.h"
#include "files/file.h"
#include "keys/keyevent.h"
#include "modes/imode.h"
#include "modes/normalmode.h"
#include "screen/draw.h"
#include "screen/iscreen.h"
#include "syntax/palette.h"
#include "text/buffer.h"
#include "text/cursorops.h"

Editor::~Editor() = default;

Editor::Editor(std::unique_ptr<Buffer> buffer)
    : _bufferView(std::move(buffer)), _cursor(_bufferView.buffer()),
      _mode(createNormalMode()) {}

void Editor::file(std::unique_ptr<IFile> file) {
    _file = std::move(file);
}

IFile *Editor::file() {
    return _file.get();
}

filesystem::path Editor::path() {
    if (_file) {
        return _file->path();
    }
    else {
        return {};
    }
}

void Editor::background(FormatType c) {
    _background = c;
}

void Editor::save() {
    if (_file) {
        _file->save(_bufferView.buffer());
        _bufferView.buffer().isChanged(false);
    }
}

void Editor::load() {
    if (_file) {
        _file->load(_bufferView.buffer());
        _bufferView.buffer().isChanged(false);
    }
}

void Editor::undo() {
    _history.undo(buffer());
}

void Editor::redo() {
    _history.redo(buffer());
}

Buffer &Editor::buffer() {
    return _bufferView.buffer();
}

void Editor::buffer(std::unique_ptr<Buffer> buffer) {
    _bufferView.buffer(std::move(buffer));
}

BufferView &Editor::bufferView() {
    return _bufferView;
}

Cursor Editor::cursor() const {
    return _cursor;
}

Cursor Editor::cursor(Cursor c, bool deselect) {
    _cursor = c;
    const auto &lines = _bufferView.buffer().lines();
    if (_cursor.y() > lines.size()) {
        _cursor.y(lines.size());
    }
    fitCursor();
    if (deselect) {
        _selectionAnchor.reset();
    }

    return _cursor;
}

Cursor Editor::cursor(Position position) {
    return cursor({buffer(), position});
}

void Editor::anchor(Cursor cursor) {
    _selectionAnchor = {cursor};
}

void Editor::clearSelection() {
    _selectionAnchor = {};
}

CursorRange Editor::selection() {
    if (_selectionAnchor) {
        return {*_selectionAnchor, _cursor};
    }
    else {
        return {_cursor};
    }
}

void Editor::selection(CursorRange range) {
    anchor(range.begin());
    cursor(range.end());
}

void Editor::mode(std::shared_ptr<IMode> mode) {
    if (_mode) {
        _mode->exit(*this);
    }
    _mode = move(mode);
    _history.commit(buffer());
    _mode->start(*this);
}

IMode &Editor::mode() {
    return *_mode;
}

void Editor::showLines(bool value) {
    _bufferView.showLines(value);
}

bool Editor::keyPress(std::shared_ptr<IScope> env) {
    if (_mode) {
        if (_mode->keyPress(env)) {
            _history.commit(buffer());
            return true;
        }
        else {
            return false;
        }
    }

    return false;
}

void Editor::updateCursor(IScreen &screen) const {
    constexpr auto debug = true;

    if (debug) {
        screen.draw(1,
                    screen.height() - 1,
                    std::to_string(_cursor.y()) + ", " +
                        std::to_string(_cursor.x()));

        screen.draw(10, screen.height() - 1, _mode->name());
    }

    // Make the cursor appear as it is on the line but can stay on same x
    // position between longer lines
    auto tmpCursor = fix(_cursor);

    screen.cursor(_bufferView.x() + _bufferView.numberWidth() + tmpCursor.x(),
                  _bufferView.y() + tmpCursor.y() - _bufferView.yScroll());
}

void Editor::draw(IScreen &screen) {
    fillRect(screen, {' ', IPalette::standard}, x(), y(), width(), height());

    _bufferView.draw(screen);

    if (auto sel = selection(); !sel.empty()) {
        _bufferView.drawSpecial(screen, sel, IPalette::selection);
    }
}

void Editor::height(size_t value) {
    View::height(value);
    _bufferView.height(value);
}

void Editor::width(size_t value) {
    View::width(value);
    _bufferView.width(value);
}

size_t Editor::width() const {
    return View::width();
}

size_t Editor::height() const {
    return View::height();
}

void Editor::x(size_t x) {
    View::x(x);
    _bufferView.x(x);
}

void Editor::y(size_t y) {
    View::y(y);
    _bufferView.y(y);
}

size_t Editor::x() const {
    return View::x();
}

size_t Editor::y() const {
    return View::y();
}

void Editor::fitCursor() {
    bufferView().fitPosition(_cursor);
}
