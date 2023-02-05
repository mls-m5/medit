#pragma once

#ifdef __EMSCRIPTEN__

#include "screen/iinput.h"
#include "screen/iscreen.h"
#include "syntax/palette.h"
#include <memory>
#include <queue>

class HtmlScreen : public IScreen, public IInput {
public:
    HtmlScreen();
    ~HtmlScreen();

    // @see IScreen
    void draw(size_t x, size_t y, const FString &str) override;
    // @see IScreen
    void refresh() override;
    // @see IScreen
    void clear() override;
    // @see IScreen
    void cursor(size_t x, size_t y) override;
    // @see IScreen
    size_t x() const override;
    // @see IScreen
    size_t y() const override;
    // @see IScreen
    size_t width() const override;
    // @see IScreen
    size_t height() const override;
    // @see IScreen
    void title(std::string title) override;
    // @see IScreen
    void palette(const Palette &) override;
    // @see IScreen
    size_t addStyle(const Color &foreground,
                    const Color &background,
                    size_t index) override;
    // @see IScreen
    void cursorStyle(CursorStyle) override;

    // @see IInput
    void subscribe(CallbackT f) override;

    // @see IInput
    void unsubscribe() override;

    void sendKeyEvent(KeyEvent event);

private:
    struct Grid;

    std::unique_ptr<Grid> _grid;

    Palette _palette;

    CallbackT _callback;
};

#endif
