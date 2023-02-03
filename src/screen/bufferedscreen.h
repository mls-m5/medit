#pragma once

#include "screen/iinput.h"
#include "screen/iscreen.h"
#include <memory>
#include <thread>

class BufferedScreen : public IScreen, public IInput {
    IScreen *_backend;
    IInput *_input;
    struct Canvas;
    std::unique_ptr<Canvas> _canvas;
    std::thread::id _threadId;

    void forceThread();

public:
    BufferedScreen(IScreen *backend, IInput *keysink);
    ~BufferedScreen();

    //! @see IScreen
    void draw(size_t x, size_t y, const FString &str) override;
    //! @see IScreen
    void refresh() override;
    //! @see IScreen
    void clear() override;
    //! @see IScreen
    void cursor(size_t x, size_t y) override;
    //! @see IScreen
    size_t x() const override;
    //! @see IScreen
    size_t y() const override;
    //! @see IScreen
    size_t width() const override;
    //! @see IScreen
    size_t height() const override;
    //! @see IScreen
    void title(std::string title) override;
    //! @see IScreen
    const IPalette &palette() const override;
    //! @see IScreen
    IPalette &palette() override;
    //! @see IScreen
    size_t addStyle(const Color &foreground,
                    const Color &background,
                    size_t index) override;
    //! @see IScreen
    void cursorStyle(CursorStyle) override;

    //! @see IInput
    Event getInput() override;
};
