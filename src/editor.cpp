
#include "editor.h"

void Editor::keyPress(int c) {
    if (c == 263) {
        _buffer.erase(_cursor);
    }
    else {
        _cursor = _buffer.insert(c, _cursor);
    }
}
