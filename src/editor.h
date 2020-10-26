#pragma once

#include "buffer.h"
#include "meditfwd.h"

class Editor {
private:
    Buffer _buffer;
    Cursor _cursor;
    //    IMode *mode = 0;

public:
    const auto &buffer() const {
        return _buffer;
    }

    void keyPress(const KeyEvent &key);
};
