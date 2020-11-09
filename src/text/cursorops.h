#pragma once

#include "meditfwd.h"
#include "text/cursor.h"
#include "text/utf8char.h"

//! Check if cursor is inside buffer range for x and y
[[nodiscard]] bool isValid(Cursor);
[[nodiscard]] Cursor fix(Cursor);
[[nodiscard]] Cursor right(Cursor, bool allowLineChange = true);
[[nodiscard]] Cursor left(Cursor, bool allowLineChange = true);
[[nodiscard]] Cursor home(Cursor);
[[nodiscard]] Cursor end(Cursor);

//! Goto the beginning of the current word
[[nodiscard]] Cursor wordBegin(Cursor);
[[nodiscard]] Cursor wordEnd(Cursor);

Cursor insert(Utf8Char, Cursor);

//! Remove the character before the cursor
Cursor erase(Cursor);

Cursor deleteLine(Cursor);

//! Join the line with the one next under
//! @return cursor where the strings are joined
Cursor join(Cursor);

Cursor split(Cursor);

//! Used for auto indentation
Cursor copyIndentation(Cursor, std::string autoIndentString = "    ");

Utf8Char content(Cursor);
