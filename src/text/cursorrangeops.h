#pragma once

#include "cursorrange.h"
#include "fstring.h"
#include <iosfwd>

//! Get the text content in a range
std::vector<FString> content(CursorRange range);

Cursor erase(CursorRange range);

void format(CursorRange, FormatType);

bool operator==(CursorRange, std::string_view);

std::ostream &operator<<(std::ostream &, CursorRange);
