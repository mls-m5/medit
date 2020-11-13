
#include "cursor.h"
#include "buffer.h"
#include "text/cursorops.h"

Cursor &Cursor::operator++() {
    return *this = right(*this);
}

FChar *Cursor::operator*() {
    return contentPtr(*this);
}
