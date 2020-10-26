
#include "editor.h"
#include "buffer.h"
#include "keys/keyevent.h"
#include "modes/imode.h"
#include "screen/iscreen.h"

void Editor::keyPress(const KeyEvent &event) {
    if (_mode) {
        _mode->keyPress(event, *this);
    }
}

void Editor::updateCursor(IScreen &screen) const {
    constexpr auto debug = true;
    if (debug) {
        screen.draw(1,
                    screen.height() - 1,
                    std::to_string(_cursor.x) + ", " +
                        std::to_string(_cursor.y));
    }

    screen.cursor(x() + _cursor.x, y() + _cursor.y);
}
