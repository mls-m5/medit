#pragma once

#include "core/threadvalidation.h"
#include "screen/iscreen.h"
#include <memory>

class BufferedScreen : public IScreen {
    IScreen *_backend;
    struct Canvas;
    std::unique_ptr<Canvas> _canvas;

    ThreadValidation _tv{"BufferedScreen"};

public:
    BufferedScreen(const BufferedScreen &) = delete;
    BufferedScreen(BufferedScreen &&) = delete;
    BufferedScreen &operator=(const BufferedScreen &) = delete;
    BufferedScreen &operator=(BufferedScreen &&) = delete;
    BufferedScreen(IScreen *backend);
    ~BufferedScreen() override;

    //! @see IScreen
    void draw(size_t x, size_t y, FStringView str) override;
    //! @see IScreen
    void refresh() override;
    //! @see IScreen
    void clear() override;
    //! @see IScreen
    void cursor(size_t x, size_t y) override;
    //! @see IScreen
    void title(std::string title) override;
    //! @see IScreen
    void palette(const Palette &palette) override;
    //! @see IScreen
    size_t addStyle(const Color &foreground,
                    const Color &background,
                    size_t index) override;
    //! @see IScreen
    void cursorStyle(CursorStyle) override;

    //! @see IInput
    void subscribe(CallbackT f) override;

    //! @see IInput
    void unsubscribe() override;

    //! @see IScreen
    std::string clipboardData() override;

    //! @see IScreen
    void clipboardData(std::string) override;
};
