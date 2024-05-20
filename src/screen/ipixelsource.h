#pragma once

#if __has_include("sdlpp/surface.hpp")
#include "sdlpp/surface.hpp"
#define HAS_READ_PIXELS
#endif

class IPixelSource {
public:
    using PixelReturnType =
#ifdef HAS_READ_PIXELS
        sdl::Surface;
#else
        void;
#endif

    virtual PixelReturnType readPixels() = 0;
};
