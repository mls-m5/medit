#ifdef __EMSCRIPTEN__

#include "screen/htmlscreen.h"
#include "syntax/color.h"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <iostream>

enum JsEventType {
    Text = 1,
    FKey = 2,
};

EM_JS(void,
      drawCell,
      (int x,
       int y,
       int c1,
       int fgr,
       int fgg,
       int fgb,
       int bgr,
       int bgg,
       int bgb),
      {
          cell(x, y).innerText = String.fromCharCode([c1]);
          cell(x, y).style.background = rgb(bgr, bgb, bgb);
          cell(x, y).style.color = rgb(fgr, fgb, fgb);
      });

EM_JS(void, moveCursor, (int x, int y), { moveCarret(x, y); });

namespace {

auto staticPointer = (HtmlScreen *){nullptr};

struct CellContent {
    Utf8Char c;
    Color fg = {'\0', '\0', '\0'};
    Color bg = {255, 255, 255};
    char updated = false;
};

} // namespace

EMSCRIPTEN_KEEPALIVE
extern "C" void keyCallback(int type, char c1, char c2, char c3, char c4) {
    auto str = std::string{c1, c2, c3, c4};
    auto c = Utf8Char{str.data(), str.size()};
    std::cout << c.toString() << std::endl;

    if (staticPointer) {
        if (type == 1) {
            staticPointer->sendKeyEvent(
                KeyEvent{Key::Text, {str.data(), str.size()}});
        }
    }
}

struct HtmlScreen::Grid {
    struct Style {
        Color fg;
        Color bg;
    };

    std::vector<Style> styles;
    size_t addStyle(const Color &fg, const Color &bg, size_t index) {
        if (index != std::numeric_limits<size_t>::max() &&
            index < styles.size()) {
        }
        else {
            index = styles.size();
            styles.emplace_back();
        }

        auto &style = styles.at(index);

        style = {
            {fg.r(), fg.g(), fg.b()},
            {bg.r(), bg.g(), bg.b()},
        };

        return index;
    }

    int width = 80;
    int height = 40;

    std::vector<CellContent> cells = std::vector<CellContent>(width * height);

    void draw(size_t x, size_t y, FChar c) {
        auto index = y * width + x;
        if (index >= cells.size()) {
            return;
        }
        auto &cell = cells.at(index);

        cell.c = c.c;
        auto style = [&]() -> Style & {
            if (c.f < styles.size()) {
                return styles.at(c.f);
            }
            return styles.front();
        }();
        cell.fg = style.fg;
        cell.bg = style.bg;
        cell.updated = true;
    }

    void refresh() {
        size_t index = 0;
        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width; ++x) {
                //                if (auto &cell = cells.at(index);
                //                !cell.updated) {
                auto &cell = cells.at(index);
                cell.updated = false;
                drawCell(x,
                         y,
                         cell.c[0],
                         cell.fg.r(),
                         cell.fg.g(),
                         cell.fg.b(),
                         cell.bg.r(),
                         cell.bg.g(),
                         cell.bg.b());
                //                }

                ++index;
            }
        }
    }
};

HtmlScreen::HtmlScreen()
    : _grid(std::make_unique<Grid>()) {
    staticPointer = this;
}

HtmlScreen::~HtmlScreen() = default;

void HtmlScreen::draw(size_t x, size_t y, FStringView str) {
    for (auto c : str) {
        _grid->draw(x, y, c);
        ++x;
    }
}

void HtmlScreen::refresh() {
    _grid->refresh();
}

void HtmlScreen::clear() {
    EM_ASM(clearScreen());
}

void HtmlScreen::cursor(size_t x, size_t y) {
    moveCursor(x, y);
}

size_t HtmlScreen::x() const {
    return 0;
}

size_t HtmlScreen::y() const {
    return 0;
}

size_t HtmlScreen::width() const {
    return _grid->width;
}

size_t HtmlScreen::height() const {
    return _grid->height;
}

void HtmlScreen::title(std::string title) {}

void HtmlScreen::palette(const Palette &palette) {
    _palette = palette;
}

size_t HtmlScreen::addStyle(const Color &foreground,
                            const Color &background,
                            size_t index) {
    return _grid->addStyle(foreground, background, index);
}

void HtmlScreen::cursorStyle(CursorStyle) {}

void HtmlScreen::subscribe(CallbackT f) {
    _callback = f;
}

void HtmlScreen::unsubscribe() {
    _callback = {};
}

void HtmlScreen::sendKeyEvent(KeyEvent event) {
    _callback({event});
}

#endif
