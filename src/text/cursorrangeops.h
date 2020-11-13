#pragma once

#include "cursorrange.h"
#include "fstring.h"

//! Get the text content in a range
std::vector<FString> content(CursorRange range);

Cursor erase(CursorRange range);

void format(CursorRange, FormatType);
