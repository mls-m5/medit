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

size_t HtmlScreen::x() const {}

size_t HtmlScreen::y() const {}

size_t HtmlScreen::width() const {}

size_t HtmlScreen::height() const {}

void HtmlScreen::title(std::string title) {}

const IPalette &HtmlScreen::palette() const {}

IPalette &HtmlScreen::palette() {}

size_t HtmlScreen::addStyle(const Color &foreground,
                            const Color &background,
                            size_t index) {}

void HtmlScreen::cursorStyle(CursorStyle) {}

KeyEvent HtmlScreen::getInput() {}

#endif
