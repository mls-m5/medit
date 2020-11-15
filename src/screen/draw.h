#pragma once

#include "meditfwd.h"

void draw(IScreen &, size_t x, size_t y, const FString &);

void fillRect(
    IScreen &, const FChar &, size_t x, size_t y, size_t width, size_t height);

void fillRect(IScreen &, const FChar &, const IView &view);
