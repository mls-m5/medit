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
    void title(std::string title) override;

    const IPalette &palette() const override {
        return _palette;
    }

    IPalette &palette() override {
        return _palette;
    }

    //! IInput
    Event getInput() override;

    size_t addStyle(const Color &foreground,
                    const Color &background,
                    size_t index = std::numeric_limits<size_t>::max()) override;

    void cursorStyle(CursorStyle) override;

private:
    struct Style;
    std::vector<Style> _styles;
    size_t _lastStyle = 16;
    Palette _palette;
};
