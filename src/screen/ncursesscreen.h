#pragma once

#ifndef __EMSCRIPTEN__

#include "core/threadvalidation.h"
#include "screen/iscreen.h"
#include "syntax/palette.h"
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

class NCursesScreen : public IScreen {
public:
    void draw(size_t x, size_t y, FStringView str) override;

    void refresh() override;
    void clear() override;
    void cursor(size_t x, size_t y) override;

    NCursesScreen();
    ~NCursesScreen() override;

    void subscribe(CallbackT f) override;

    void unsubscribe() override {
        _callback = {};
    }

    //    size_t x() const override;
    //    size_t y() const override;
    size_t width() const;
    size_t height() const;
    void title(std::string title) override;

    void palette(const Palette &palette) override;

    void cursorStyle(CursorStyle) override;

    std::string clipboardData() override {
        return "";
    }

    void clipboardData(std::string) override {}

private:
    Event getInput();

    void init();

    bool _hasColors = true;

    Palette _palette;

    size_t _tabWidth = 3;

    size_t _lastStyle = 16;
    size_t _lastColor = 16;

    CursorStyle _currentCursor = CursorStyle::Block;

    ThreadValidation _tv{"ncurses thread"};

    bool _isRunning = false;

    std::thread _ncursesThread;
    std::mutex _mutex;

    CallbackT _callback;

    std::deque<std::function<void()>> _threadQueue;

    //! @see IScreen interface
    size_t addStyle(const Color &fg, const Color &bg, size_t index) override;

    void loop();
};

#endif
