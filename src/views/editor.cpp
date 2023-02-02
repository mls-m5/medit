#include "views/editor.h"
#include "files/ifile.h"
#include "modes/imode.h"
#include "modes/normalmode.h"
#include "screen/draw.h"
#include "screen/iscreen.h"
#include "syntax/ipalette.h"
#include "text/buffer.h"
#include "text/cursorops.h"

Editor::~Editor() = default;

Editor::Editor(std::shared_ptr<Buffer> buffer)
    : _bufferView(std::move(buffer))
    , _cursor(_bufferView.buffer())
    , _mode(createNormalMode()) {}

IFile *Editor::file() {
    return buffer().file();
}

filesystem::path Editor::path() {
    return buffer().path();
}

void Editor::background(FormatType c) {
    _background = c;
}

void Editor::save() {
    buffer().save();
}

void Editor::load() {
    buffer().load();
}

void Editor::undo() {
    buffer().history().undo();
}

void Editor::redo() {
    buffer().history().redo();
}

Buffer &Editor::buffer() {
    return _bufferView.buffer();
}

void Editor::buffer(std::shared_ptr<Buffer> buffer) {
    _cursor = Cursor{*buffer};
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
    const auto &lines = buffer().lines();
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
    _mode = std::move(mode);
    buffer().history().markMajor();
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
            //            buffer().history().markMajor();
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
                    std::to_string(_cursor.y() + 1) + ", " +
                        std::to_string(_cursor.x() + 1));

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
