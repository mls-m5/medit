#pragma once

#include "cursorrange.h"
#include "fstring.h"
#include <iosfwd>

//! Get the text content in a range
[[nodiscard]] std::vector<FString> content(CursorRange range);

std::string toString(CursorRange range);

Cursor erase(CursorRange range);

void format(CursorRange, FormatType);

bool operator==(CursorRange, std::string_view);

std::ostream &operator<<(std::ostream &, CursorRange);

[[nodiscard]] CursorRange fix(CursorRange);

[[nodiscard]] CursorRange word(Cursor);
[[nodiscard]] CursorRange line(Cursor);
[[nodiscard]] CursorRange inner(Cursor, Utf8Char start, Utf8Char stop);

/// Apply simple text change and return the beginning of the new text
Cursor apply(const BufferEdit &edit);
