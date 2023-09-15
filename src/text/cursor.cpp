
#include "cursor.h"
#include "fchar.h"
#include "text/cursorops.h"

Cursor &Cursor::operator++() {
    return *this = right(*this);
}

FChar Cursor::operator*() {
    return content(*this);
}
