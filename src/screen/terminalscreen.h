#pragma once

#include "screen/iinput.h"
#include "screen/iscreen.h"

class TerminalScreen : public IScreen, public IInput {
    int _width = 0;
    int _height = 0;

public:
    void draw(size_t x, size_t y, FString str) override;

    void refresh() override;

    TerminalScreen();

    KeyEvent getInput() override;
};
