
#include "editor.h"
#include "keys/keyevent.h"

void Editor::keyPress(const KeyEvent &event) {
    if (event.key == 263) {
        _buffer.erase(_cursor);
    }
    else {
        _cursor = _buffer.insert(event.key, _cursor);
    }
}
