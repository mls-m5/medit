#pragma once

#include "screen/iscreen.h"
#include "syntax/palette.h"
#include "text/fstringview.h"

class LinuxTerminalScreen : public IScreen {
public:
    LinuxTerminalScreen();
    LinuxTerminalScreen(const LinuxTerminalScreen &) = delete;
    LinuxTerminalScreen(LinuxTerminalScreen &&) = delete;
    LinuxTerminalScreen &operator=(const LinuxTerminalScreen &) = delete;
    LinuxTerminalScreen &operator=(LinuxTerminalScreen &&) = delete;

    ~LinuxTerminalScreen() override;

    //! IScreen
    void draw(size_t x, size_t y, FStringView) override;
    void refresh() override;
    void clear() override;
    void cursor(size_t x, size_t y) override;
    //    size_t x() const override;
    //    size_t y() const override;
    //    size_t width() const override;
    //    size_t height() const override;
    void title(std::string title) override;

    //! IInput
    Event getInput();

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
