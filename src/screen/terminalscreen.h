#pragma once

#include "screen/iinput.h"
#include "screen/iscreen.h"

class TerminalScreen : public IScreen, public IInput {
public:
    void draw(size_t x, size_t y, FString str) override;

    void refresh() override;
    void clear() override;
    void cursor(size_t x, size_t y) override;

    TerminalScreen();

    KeyEvent getInput() override;

    size_t x() const override;
    size_t y() const override;
    size_t width() const override;
    size_t height() const override;
};
