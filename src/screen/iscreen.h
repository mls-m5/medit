#pragma once

#include "keys/event.h"
#include "meditfwd.h"
#include "screen/cursorstyle.h"
#include "text/fstringview.h"
#include <limits>
#include <string>

class IScreen {
public:
    IScreen(const IScreen &) = default;
    IScreen(IScreen &&) = delete;
    IScreen &operator=(const IScreen &) = default;
    IScreen &operator=(IScreen &&) = delete;
    IScreen() = default;
    virtual ~IScreen() = default;

    virtual void draw(size_t x, size_t y, FStringView str) = 0;
    virtual void refresh() = 0;
    virtual void clear() = 0;
    virtual void cursor(size_t x, size_t y) = 0;

    virtual void title(std::string title) = 0;
    virtual void palette(const Palette &) = 0;

    //! Try to request clip board data from clients computer
    //! Note that clipboard data also can be sent by a clip board paste event
    virtual std::string clipboardData() = 0;

    //! Set clipboard data on clients computer
    virtual void clipboardData(std::string) = 0;

    //! @param index should be left default for most cases
    //! set index to place new style on, otherwise a new location is picked
    //! It's only used internally between screen and Palette
    //! TODO: Remove this, make the screen read from the palette instoad
    virtual size_t addStyle(
        const Color &foreground,
        const Color &background,
        size_t index = std::numeric_limits<size_t>::max()) = 0;

    virtual void cursorStyle(CursorStyle) = 0;

    using EventListT = std::vector<Event>;
    using CallbackT = std::function<void(EventListT)>;

    //! Subscribe to user input
    virtual void subscribe(CallbackT f) = 0;

    virtual void unsubscribe() = 0;
};
