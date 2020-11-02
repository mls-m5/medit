#pragma once

#include "screen/iinput.h"
#include "screen/iscreen.h"

class NCursesScreen : public IScreen, public IInput {
public:
    void draw(size_t x, size_t y, const FString &str) override;

    void refresh() override;
    void clear() override;
    void cursor(size_t x, size_t y) override;

    NCursesScreen();
    ~NCursesScreen() override;

    KeyEvent getInput() override;

    size_t x() const override;
    size_t y() const override;
    size_t width() const override;
    size_t height() const override;

private:
    size_t _tabWidth = 3;
};
