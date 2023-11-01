#include "utf8charops.h"

bool isSpace(Utf8Char c) {
    return (c == '\t' || c == ' ' || c == '\n' || c == '\r');
}
