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

filesystem::path Editor::path() {
    if (_file) {
        return _file->path();
    }
    else {
        return {};
    }
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

void Editor::mode(std::shared_ptr<IMode> mode) {
    if (_mode) {
        _mode->exit(*this);
    }
    _mode = move(mode);
    _history.commit(buffer());
    _mode->start(*this);
}

bool Editor::keyPress(IEnvironment &env) {
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

// void Editor::updatePalette(const IPalette &palette) {
//    _background = palette.getFormat(standardFormatName);
//}

void Editor::draw(IScreen &screen) {
    fillRect(screen, {' ', IPalette::standard}, x(), y(), width(), height());

    _bufferView.draw(screen);

    if (auto sel = selection(); !sel.empty()) {
        _bufferView.drawSpecial(screen, sel, IPalette::selection);
    }
}

void Editor::fitCursor() {
    bufferView().fitPosition(_cursor);
}
