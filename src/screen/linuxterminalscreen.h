#pragma once

#include "screen/iinput.h"
#include "screen/iscreen.h"
#include "syntax/palette.h"

class LinuxTerminalScreen : public IScreen, public IInput {
public:
    LinuxTerminalScreen();
    ~LinuxTerminalScreen();

    //! IScreen
    void draw(size_t x, size_t y, const FString &str) override;
    void refresh() override;
    void clear() override;
    void cursor(size_t x, size_t y) override;
    size_t x() const override;
    size_t y() const override;
    size_t width() const override;
    size_t height() const override;

    const IPalette &palette() const override {
        return _palette;
    }

    IPalette &palette() override {
        return _palette;
    }

    //! IInput
    KeyEvent getInput() override;

private:
    Palette _palette;
};
