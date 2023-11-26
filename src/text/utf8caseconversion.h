#pragma once

#include "fstring.h"
#include "utf8char.h"

Utf8Char toLower(Utf8Char);
Utf8Char toUpper(Utf8Char);
Utf8Char toggleCase(Utf8Char);

FString toLower(FString);
FString toUpper(FString);
FString toggleCase(FString);

bool isAlpha(Utf8Char);
bool isAlNum(Utf8Char);
