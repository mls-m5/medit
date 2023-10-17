#pragma once

#ifndef __EMSCRIPTEN__

#include "screen/iscreen.h"
#include "syntax/palette.h"
#include "text/fstringview.h"
#include <memory>
#include <mutex>
#include <thread>

#if __has_include("sdlpp/surface.hpp")
#include "sdlpp/surface.hpp"
#define HAS_READ_PIXELS
#endif

/// The rendering and gui is running on one thread and assumes the application
/// calls from one single thread
class GuiScreen : public IScreen {
public:
    void draw(size_t x, size_t y, FStringView str) override;

    void refresh() override;
    void clear() override;
    void cursor(size_t x, size_t y) override;

    GuiScreen();
    ~GuiScreen() noexcept override;

    void subscribe(CallbackT f) override;

    void unsubscribe() override;

    size_t width() const;
    size_t height() const;
    void title(std::string title) override;

    void palette(const Palette &palette) override {
        _palette = palette;
    }

    void cursorStyle(CursorStyle) override;

    size_t addStyle(const Color &fg, const Color &bg, size_t index) override;

    std::string clipboardData() override;
    void clipboardData(std::string) override;

#ifdef HAS_READ_PIXELS
    sdl::Surface readPixels();
#else
    void readPixels() {
        static_assert("remember to link to sdlpp");
    }
#endif

private:
    Palette _palette;

    std::thread _thread;

    struct Buffer;
    std::unique_ptr<Buffer> _buffer;

    std::mutex _mutex;
};

#endif
