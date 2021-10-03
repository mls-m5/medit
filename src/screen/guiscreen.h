#pragma once

#include "screen/iinput.h"
#include "screen/iscreen.h"
#include "syntax/palette.h"
#include <memory>
#include <mutex>
#include <thread>

class GuiScreen : public IScreen, public IInput {
public:
    void draw(size_t x, size_t y, const FString &str) override;

    void refresh() override;
    void clear() override;
    void cursor(size_t x, size_t y) override;

    GuiScreen();
    ~GuiScreen() noexcept override;

    KeyEvent getInput() override;

    size_t x() const override;
    size_t y() const override;
    size_t width() const override;
    size_t height() const override;
    void title(std::string title) override;

    const IPalette &palette() const override {
        return _palette;
    }

    IPalette &palette() override {
        return _palette;
    }

    void cursorStyle(CursorStyle) override;

    size_t addStyle(const Color &fg, const Color &bg, size_t index) override;

private:
    Palette _palette;

    struct Buffer;
    std::unique_ptr<Buffer> _buffer;
};
