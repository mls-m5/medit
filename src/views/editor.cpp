#include "views/editor.h"
#include "keys/keyevent.h"
#include "modes/imode.h"
#include "screen/iscreen.h"
#include "text/buffer.h"

Editor::~Editor() = default;

void Editor::keyPress(IEnvironment &env) {
    if (_mode) {
        _mode->keyPress(env);
    }
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

    screen.draw(x() + 5, y(), _file->representation());

    screen.cursor(_bufferView.x() + _bufferView.numberWidth() + _cursor.x(),
                  _bufferView.y() + _cursor.y());
}

void Editor::draw(IScreen &screen) {
    _bufferView.draw(screen);
}
