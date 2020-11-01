#pragma once

#include "text/cursor.h"
#include "text/utf8char.h"

//! Check if cursor is inside buffer range for x and y
[[nodiscard]] bool isValid(Cursor cursor);
[[nodiscard]] Cursor fix(Cursor cursor);
[[nodiscard]] Cursor right(Cursor cursor, bool allowLineChange = true);
[[nodiscard]] Cursor left(Cursor cursor, bool allowLineChange = true);
[[nodiscard]] Cursor home(Cursor cursor);
[[nodiscard]] Cursor end(Cursor cursor);

Cursor insert(Utf8Char, Cursor);

//! Remove the character before the cursor
Cursor erase(Cursor);

Cursor deleteLine(Cursor);

//! Join the line with the one next under
//! @return cursor where the strings are joined
Cursor join(Cursor);

Cursor split(Cursor);
