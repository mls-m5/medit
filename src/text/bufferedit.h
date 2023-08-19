#pragma once

#include "cursor.h"
#include "fstring.h"
#include <vector>

class BufferEdit {
public:
    // TODO: Maybe create a better stringtype (basic_string does not work)
    FString from;
    FString to;
    Cursor position;

    void trim();

    bool empty() {
        return from.empty() && to.empty();
    }
};

BufferEdit revert(BufferEdit e);
