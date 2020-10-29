#pragma once

#include "text/buffer.h"

void colorize(FString &line);

inline void colorize(Buffer &buffer) {
    for (auto &line : buffer.lines()) {
        colorize(line);
    }
}
