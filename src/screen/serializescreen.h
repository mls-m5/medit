#pragma once

#include "iconnection.h"
#include "screen/iscreen.h"

//! Screen used on server to serialize screen calls to binary form
class SerializeScreen : public IScreen {
    SerializeScreen(IConnection &connection);

    // IScreen interface
public:
    void draw(size_t x, size_t y, const FString &str);
    void refresh();
    void clear();
    void cursor(size_t x, size_t y);
    size_t x() const;
    size_t y() const;
    size_t width() const;
    size_t height() const;
    void title(std::string title);
    const IPalette &palette() const;
    IPalette &palette();
    size_t addStyle(const Color &foreground,
                    const Color &background,
                    size_t index);
    void cursorStyle(CursorStyle);
};
