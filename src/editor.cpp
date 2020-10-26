
#include "editor.h"
#include "buffer.h"
#include "keys/keyevent.h"
#include "screen/iscreen.h"

void Editor::keyPress(const KeyEvent &event) {
    if (event.key == Key::Backspace) {
        _cursor = buffer().erase(_cursor);
    }
    else if (event.key == Key::Left) {
        _cursor = buffer().prev(_cursor);
    }
    else if (event.key == Key::Right) {
        _cursor = buffer().next(_cursor);
    }
    else if (event.key == Key::Down) {
        //        _cursor = buffer().fixCursor({_cursor.x, _cursor.y + 1});
        _cursor.y += 1;
    }
    else if (event.key == Key::Up) {
        if (_cursor.y == 0) {
            return;
        }
        _cursor.y -= 1;
        //        _cursor = buffer().fixCursor({_cursor.x, _cursor.y - 1});
    }
    else if (event.key == Key::Text) {
        _cursor = buffer().insert(event.symbol, _cursor);
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
