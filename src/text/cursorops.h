#pragma once

#include "meditfwd.h"
#include "text/cursor.h"
#include "text/utf8char.h"
#include <optional>

//! Check if cursor is inside buffer range for x and y
[[nodiscard]] bool isValid(Cursor);
[[nodiscard]] Cursor fix(Cursor);
[[nodiscard]] Cursor right(Cursor, bool allowLineChange = true);
[[nodiscard]] Cursor left(Cursor, bool allowLineChange = true);
[[nodiscard]] Cursor up(Cursor);
[[nodiscard]] Cursor down(Cursor);
[[nodiscard]] Cursor home(Cursor);
[[nodiscard]] Cursor end(Cursor);

//! Goto the beginning of the current word
[[nodiscard]] Cursor wordBegin(Cursor);
[[nodiscard]] Cursor wordEnd(Cursor);

//! Goto the beginning of the next word
//! Used to implement vim motion w
[[nodiscard]] Cursor nextWord(Cursor, bool allowLineChange = true);

//! Find stuff
[[nodiscard]] std::optional<Cursor> find(Cursor,
                                         Utf8Char,
                                         bool allowLineChange = false);
[[nodiscard]] std::optional<Cursor> rfind(Cursor,
                                          Utf8Char,
                                          bool allowLineChange = false);

// TODO: change this order, cursor should really be first
Cursor insert(Utf8Char, Cursor);
Cursor insert(Cursor, FString);

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

//! For autocompleteon find the first character of the word to be completed
Cursor autocompleteWordBegin(const Cursor cursor);

Cursor apply(BufferEdit edit);
