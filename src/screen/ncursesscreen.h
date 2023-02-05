#pragma once

#ifndef __EMSCRIPTEN__

#include "screen/iinput.h"
#include "screen/iscreen.h"
#include "syntax/palette.h"
#include <mutex>
#include <thread>

class NCursesScreen : public IScreen, public IInput {
public:
    void draw(size_t x, size_t y, const FString &str) override;

    void refresh() override;
    void clear() override;
    void cursor(size_t x, size_t y) override;

    NCursesScreen();
    ~NCursesScreen() override;

    Event getInput() override;

    size_t x() const override;
    size_t y() const override;
    size_t width() const override;
    size_t height() const override;
    void title(std::string title) override;

    const IPalette &palette() const override {
        return _palette;
    }

    void palette(const Palette &palette) override {
        _palette = palette;
        _palette.update(*this);
    }

    void cursorStyle(CursorStyle) override;

private:
    void init();

    void forceThread();

    bool _hasColors = true;

    Palette _palette;

    size_t _tabWidth = 3;

    size_t _lastStyle = 16;
    size_t _lastColor = 16;

    CursorStyle _currentCursor = CursorStyle::Block;

    std::thread::id _threadId;

    // Without a mutex ncurses will segfault at some random times
    // when writing to the screen and getting input simultaneously
    //    std::mutex _accessMutex;

    //! @see IScreen interface
    size_t addStyle(const Color &fg, const Color &bg, size_t index) override;
};

#endif
