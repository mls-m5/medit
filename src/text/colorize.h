#pragma once

#include "text/buffer.h"

void colorize(FString &line);

inline void colorize(Buffer &buffer) {
    for (size_t i = 0; i < buffer.lines().size(); ++i) {
        colorize(buffer.lineAt(i));
    }
    //    for (auto &line : buffer.lines()) {
    //        colorize(line);
    //    }
}
