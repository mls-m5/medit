#include "views/editor.h"
#include "files/file.h"
#include "keys/keyevent.h"
#include "modes/imode.h"
#include "screen/iscreen.h"
#include "text/buffer.h"
#include "text/cursorops.h"

Editor::~Editor() = default;

Editor::Editor(std::unique_ptr<Buffer> buffer)
    : _bufferView(std::move(buffer)), _cursor(_bufferView.buffer()) {}

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
        _bufferView.buffer().changed(false);
    }
}

void Editor::load() {
    if (_file) {
        _file->load(_bufferView.buffer());
        _bufferView.buffer().changed(false);
    }
}

bool Editor::keyPress(IEnvironment &env) {
    if (_mode) {
        return _mode->keyPress(env);
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

    screen.cursor(_bufferView.x() + _bufferView.numberWidth() + _cursor.x(),
                  _bufferView.y() + _cursor.y() - _bufferView.yScroll());
}

void Editor::draw(IScreen &screen) {
    _bufferView.draw(screen);
}

void Editor::fitCursor() {
    bufferView().fitPosition(_cursor);
}
