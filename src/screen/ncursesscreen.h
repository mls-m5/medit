#pragma once

#include "screen/iinput.h"
#include "screen/iscreen.h"
#include "syntax/palette.h"

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

    const IPalette &palette() const override {
        return _palette;
    }

    IPalette &palette() override {
        return _palette;
    }

private:
    void init();

    bool _hasColors = true;

    Palette _palette;

    size_t _tabWidth = 3;

    size_t _lastStyle = 16;
    size_t _lastColor = 16;

    //! @see IScreen interface
    size_t addStyle(const Color &fg, const Color &bg, size_t index) override;
};
