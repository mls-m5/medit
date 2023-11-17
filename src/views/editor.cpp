#include "views/editor.h"
#include "files/ifile.h"
#include "modes/imode.h"
#include "modes/normalmode.h"
#include "screen/draw.h"
#include "screen/iscreen.h"
#include "script/ienvironment.h"
#include "syntax/palette.h"
#include "text/buffer.h"
#include "text/cursorops.h"
#include "views/iwindow.h"
#include <optional>
#include <stdexcept>

Editor::~Editor() = default;

Editor::Editor(IView *parent, std::shared_ptr<Buffer> buffer)
    : View{parent}
    , _bufferView(parent, std::move(buffer))
    , _cursor(_bufferView.buffer())
    , _mode(createNormalMode()) {}

IFile *Editor::file() {
    return buffer().file();
}

std::filesystem::path Editor::path() {
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

void Editor::undoMajor() {
    buffer().history().undoMajor();
}

void Editor::redo() {
    buffer().history().redo();
}

Buffer &Editor::buffer() {
    return _bufferView.buffer();
}

void Editor::buffer(std::shared_ptr<Buffer> buffer) {
    if (!buffer) {
        throw std::runtime_error{"Trying to set editor buffer to null"};
    }

    std::optional<SavedState> foundState;
    for (auto it = _openedBuffers.begin(); it != _openedBuffers.end();) {
        auto ptr = it->buffer.lock();
        if (!ptr) {
            it = _openedBuffers.erase(it);
        }
        else if (ptr == buffer) {
            foundState = std::move(*it);
            it = _openedBuffers.erase(it);
        }
        else {
            ++it;
        }
    }

    if (foundState) {
        restoreState(*foundState);
        return;
    }

    _openedBuffers.push_back({_bufferView.buffer().weak_from_this(),
                              _cursor.pos(),
                              {_bufferView.xScroll(), _bufferView.yScroll()}});

    _cursor = Cursor{*buffer};
    _bufferView.scroll(0, 0);
    _bufferView.buffer(std::move(buffer));
}

Cursor Editor::cursor() const {
    auto cur = fix(_cursor);
    if (_mode->shouldSelectPlusOne()) {
        return cur;
    }

    if (cur.x() == 0) {
        return cur;
    }

    cur.x(std::min(cur.x(), lineAt(cur).size() - 1));
    return cur;
}

Cursor Editor::virtualCursor() const {
    return _cursor;
}

Position Editor::cursorPosition(Position editorPos) const {
    return _bufferView.cursorPosition(editorPos);
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

    if (auto w = window()) {
        w->triggerRedraw();
    }

    return _cursor;
}

Cursor Editor::cursor(Position position) {
    return cursor({buffer(), position});
}

void Editor::selectionAnchor(Cursor cursor) {
    _selectionAnchor = {cursor};
}

std::optional<Cursor> Editor::selectionAnchor() const {
    return _selectionAnchor;
}

void Editor::clearSelection() {
    _selectionAnchor = {};
}

CursorRange Editor::selection() {
    if (_mode->isBlockSelection()) {
        if (!_selectionAnchor) {
            return {home(cursor()), end(cursor())};
        }

        auto range = CursorRange{*_selectionAnchor, cursor()};
        range.beginPosition(home(range.begin()));
        range.endPosition(right(end(range.end())));
        return range;
    }

    if (_selectionAnchor) {
        auto range = CursorRange{*_selectionAnchor, _cursor};

        range.endPosition(right(range.end(), false));

        return range;
    }
    return {_cursor};
}

bool Editor::isBlockSelection() const {
    return _mode->isBlockSelection();
}

void Editor::selection(CursorRange range) {
    selectionAnchor(range.begin());
    cursor(range.end());
}

void Editor::mode(std::shared_ptr<IMode> mode) {
    // This seems silly, but it is required when for example switching from
    // normal mode so that the virtual cursor position matches up with the
    // actual cursor position
    cursor(cursor());

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

bool Editor::keyPress(std::shared_ptr<IEnvironment> env) {
    if (_mode) {
        if (_mode->keyPress(env)) {
            if (_mode->isEverythingMajor()) {
                _bufferView.buffer().history().markMajor();
            }
            return true;
        }
        else {
            return false;
        }
    }

    return false;
}

bool Editor::mouseDown(int x, int y) {
    if (!isInside(x, y)) {
        return false;
    }

    int relX = x - this->x() - _bufferView.numberWidth();
    int relY = y - this->y();

    // TODO: It does not seem to select when clicking where there is no text
    cursor(fix(Cursor(buffer(), relX, relY + _bufferView.yScroll())));

    return true;
}

void Editor::updateCursor(IScreen &screen) const {

    // Make the cursor appear as it is on the line but can stay on same x
    // position between longer lines
    auto tmpCursor = cursor(); // fix(_cursor);

    screen.cursor(_bufferView.x() + _bufferView.numberWidth() + tmpCursor.x() -
                      _bufferView.xScroll(),
                  _bufferView.y() + tmpCursor.y() - _bufferView.yScroll());
}

void Editor::draw(IScreen &screen) {
    fillRect(screen, {' ', Palette::standard}, x(), y(), width(), height());

    _bufferView.draw(screen);

    if (auto sel = selection(); !sel.empty()) {
        _bufferView.drawSpecial(screen, sel, Palette::selection);
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
    _bufferView.fitPosition(fix(_cursor));
}

bool Editor::closeBuffer() {
    for (; !_openedBuffers.empty();) {
        auto buffer = _openedBuffers.back().buffer.lock();
        if (!buffer) {
            _openedBuffers.pop_back();
            continue;
        }
        if (buffer) {
            restoreState(std::move(_openedBuffers.back()));
            _openedBuffers.pop_back();
            return true;
        }
    }

    return false;
}

void Editor::removeEmptyUnsavedBufferHistory() {
    if (_openedBuffers.empty()) {
        return;
    }

    auto buffer = _openedBuffers.front().buffer.lock();

    /// If it is only a empty unsaved buffer at startup. Nobody will miss it
    if (buffer->empty() && !buffer->file()) {
        _openedBuffers.erase(_openedBuffers.begin());
    }
}

void Editor::restoreState(SavedState state) {
    _bufferView.buffer(state.buffer.lock());
    auto scroll = state.scroll;
    _bufferView.scroll(scroll.x(), scroll.y());
    cursor({buffer(), state.pos});
    fitCursor();
}
