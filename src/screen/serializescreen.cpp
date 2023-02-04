#include "serializescreen.h"
#include "json/json.h"

SerializeScreen::SerializeScreen(IConnection &connection) {}

void SerializeScreen::draw(size_t x, size_t y, const FString &str) {}

void SerializeScreen::refresh() {}

void SerializeScreen::clear() {}

void SerializeScreen::cursor(size_t x, size_t y) {
    Json{{"hello", "hello"}};
}

size_t SerializeScreen::x() const {}

size_t SerializeScreen::y() const {}

size_t SerializeScreen::width() const {}

size_t SerializeScreen::height() const {}

void SerializeScreen::title(std::string title) {}

const IPalette &SerializeScreen::palette() const {}

IPalette &SerializeScreen::palette() {}

size_t SerializeScreen::addStyle(const Color &foreground,
                                 const Color &background,
                                 size_t index) {}

void SerializeScreen::cursorStyle(CursorStyle) {}
