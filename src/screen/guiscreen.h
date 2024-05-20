#pragma once

#ifndef __EMSCRIPTEN__

#include "screen/ipixelsource.h"
#include "screen/iscreen.h"
#include <memory>

class IGuiScreen : public virtual IScreen, public virtual IPixelSource {
public:
    virtual void fontSize(int fontSize) = 0;

    virtual void resize(int width, int height) = 0;

    virtual size_t width() const = 0;
    virtual size_t height() const = 0;
};

std::unique_ptr<IGuiScreen> createGuiScreen();

#endif
