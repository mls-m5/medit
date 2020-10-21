#pragma once

#include "buffer.h"

class Editor {
private:
    Buffer _buffer;
    Cursor _cursor;

public:
    const auto &buffer() const {
        return _buffer;
    }

    void keyPress(int c);
};
