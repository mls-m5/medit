
#include "insertmode.h"
#include "views/editor.h"

void InsertMode::keyPress(const KeyEvent event, Editor &e) {
    if (event.key == Key::Backspace) {
        e.cursor(e.buffer().erase(e.cursor()));
    }
    else if (event.key == Key::Left) {
        e.cursor(e.buffer().prev(e.cursor()));
    }
    else if (event.key == Key::Right) {
        e.cursor(e.buffer().next(e.cursor()));
    }
    else if (event.key == Key::Down) {
        e.cursor().y += 1;
    }
    else if (event.key == Key::Up) {
        if (e.cursor().y == 0) {
            return;
        }
        e.cursor().y -= 1;
    }
    else if (event.key == Key::Text) {
        e.cursor(e.buffer().insert(event.symbol, e.cursor()));
    }
}

std::string_view InsertMode::name() {
    return "insert";
}
