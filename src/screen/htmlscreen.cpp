#ifdef __EMSCRIPTEN__

#include "screen/htmlscreen.h"
#include "emscripten.h"

struct HtmlScreen::Grid {};

HtmlScreen::HtmlScreen() = default;

HtmlScreen::~HtmlScreen() = default;

void HtmlScreen::draw(size_t x, size_t y, const FString &str) {}

void HtmlScreen::refresh() {}

void HtmlScreen::clear() {}

void HtmlScreen::cursor(size_t x, size_t y) {}

size_t HtmlScreen::x() const {
    return 0;
}

size_t HtmlScreen::y() const {
    return 0;
}

size_t HtmlScreen::width() const {
    return 10;
}

size_t HtmlScreen::height() const {
    return 10;
}

void HtmlScreen::title(std::string title) {}

const IPalette &HtmlScreen::palette() const {
    return _palette;
}

IPalette &HtmlScreen::palette() {
    return _palette;
}

size_t HtmlScreen::addStyle(const Color &foreground,
                            const Color &background,
                            size_t index) {
    return 0;
}

void HtmlScreen::cursorStyle(CursorStyle) {}

KeyEvent HtmlScreen::getInput() {
    return KeyEvent{};
}

#endif
