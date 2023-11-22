#pragma once

#include "meditfwd.h"
#include "text/cursor.h"
#include "text/fstringview.h"
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
//! @param cursorBefore is used for example "t" commands where you do not want
//! to include the last character
[[nodiscard]] std::optional<Cursor> find(Cursor,
                                         Utf8Char,
                                         bool allowLineChange = false,
                                         bool cursorBefore = false);
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

/// Get the indentation from a line
[[nodiscard]] FString indentation(Cursor cursor);

//! Used for auto indentation
Cursor copyIndentation(Cursor, std::string autoIndentString = "    ");

Cursor fixBraceIndentation(Cursor);

Utf8Char content(Cursor);

//! For autocompletion find the first character of the word to be completed
Cursor autocompleteWordBegin(const Cursor cursor);

Cursor apply(BufferEdit edit);

/// Used find mathcing, parantheses, braces etc
[[nodiscard]] std::optional<Cursor> matchingLeft(
    Cursor cursor,
    const Utf8Char start,
    const Utf8Char stop,
    bool shouldEnableNestCheck = true);

[[nodiscard]] std::optional<Cursor> matchingRight(
    Cursor cursor,
    const Utf8Char start,
    const Utf8Char stop,
    bool shouldEnableNestCheck = true);

[[nodiscard]] Cursor findAnyMatching(Cursor cursor);

/// Implementations for things like vims % command
Cursor findMatching(Cursor);

[[nodiscard]] FStringView lineAt(Cursor);

[[nodiscard]] Cursor lastNonSpaceOnLine(Cursor cursor);

[[nodiscard]] Cursor firstNonSpaceOnLine(Cursor cursor);
