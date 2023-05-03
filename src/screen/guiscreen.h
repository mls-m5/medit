#pragma once

#include "screen/iscreen.h"
#include "syntax/palette.h"
#include "text/fstringview.h"
#include <memory>
#include <mutex>
#include <thread>

#ifndef __EMSCRIPTEN__

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

    //    size_t x() const override;
    //    size_t y() const override;
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

private:
    Palette _palette;

    std::thread _thread;

    struct Buffer;
    std::unique_ptr<Buffer> _buffer;

    std::mutex _mutex;
};

#endif
