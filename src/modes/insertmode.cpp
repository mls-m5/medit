
#include "insertmode.h"
#include "script/matscript.h"
#include "views/editor.h"

void InsertMode::keyPress(const KeyEvent &event, Editor &e) {
    switch (event.key) {
    case Key::Backspace:
        e.cursor(e.buffer().erase(e.cursor()));
        break;
    case Key::Left:
        run("editor.previous", _env);
        break;
    case Key::Right:
        run("editor.next", _env);
        break;
    case Key::Down:
        run("editor.down", _env);
        break;
    case Key::Up:
        run("editor.up", _env);
        break;
    case Key::Escape:
        run("editor.normalmode", _env);
        break;
    case Key::Text:
        e.cursor(e.buffer().insert(event.symbol, e.cursor()));
        break;
    default:
        break;
    }
}

std::string_view InsertMode::name() {
    return "insert";
}
