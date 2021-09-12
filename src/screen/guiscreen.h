#pragma once

#include "matgui/application.h"
#include "matgui/window.h"
#include "screen/iinput.h"
#include "screen/iscreen.h"
#include "syntax/palette.h"
#include <memory>
#include <mutex>
#include <thread>

class GuiScreen : public IScreen, public IInput {
public:
    void draw(size_t x, size_t y, const FString &str) override;

    void refresh() override;
    void clear() override;
    void cursor(size_t x, size_t y) override;

    GuiScreen();
    ~GuiScreen() noexcept override;

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

    void cursorStyle(CursorStyle) override;

    size_t addStyle(const Color &fg, const Color &bg, size_t index) override;

private:
    static constexpr size_t constWidth = 80;
    static constexpr size_t constHeight = 24;

    bool _hasColors = true;

    Palette _palette;

    size_t _tabWidth = 3;

    size_t _lastStyle = 16;
    size_t _lastColor = 16;

    struct Buffer;
    std::unique_ptr<Buffer> _buffer;

    matgui::Application _application;
    matgui::Window _window;

    std::thread _guiThread;
    bool _isRunning = false;

    std::vector<KeyEvent> _inputQueue;
    std::mutex _inputAvailableMutex;
    std::mutex _queueLock;
};
