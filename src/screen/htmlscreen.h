#pragma once

#ifdef __EMSCRIPTEN__

#include "screen/iinput.h"
#include "screen/iscreen.h"
#include <memory>

class HtmlScreen : public IScreen, public IInput {
public:
    HtmlScreen();
    ~HtmlScreen();

    // @see IScreen
    void draw(size_t x, size_t y, const FString &str);
    // @see IScreen
    void refresh();
    // @see IScreen
    void clear();
    // @see IScreen
    void cursor(size_t x, size_t y);
    // @see IScreen
    size_t x() const;
    // @see IScreen
    size_t y() const;
    // @see IScreen
    size_t width() const;
    // @see IScreen
    size_t height() const;
    // @see IScreen
    const IPalette &palette() const;
    // @see IScreen
    IPalette &palette();
    // @see IScreen
    size_t addStyle(const Color &foreground,
                    const Color &background,
                    size_t index);
    // @see IScreen
    void cursorStyle(CursorStyle);

    // @see IInput
    KeyEvent getInput();

private:
    struct Grid;

    std::unique_ptr<Grid> _grid;
};

#endif
