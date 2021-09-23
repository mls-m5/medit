#include "guiscreen.h"
#include "SDL2/SDL_keyboard.h"
#include "files/fontlocator.h"
#include "matrixscreen.h"
#include "sdlpp/events.hpp"
#include "sdlpp/keyboard.hpp"
#include "sdlpp/render.hpp"
#include "sdlpp/window.hpp"
#include "syntax/color.h"
#include "text/position.h"
#include <array>

namespace {

size_t testX = 0;

auto keyMap = std::array<std::pair<int, Key>, 22>{{
    {SDL_SCANCODE_UNKNOWN, Key::Unknown},
    {SDL_SCANCODE_ESCAPE, Key::Escape},
    {SDL_SCANCODE_UP, Key::Up},
    {SDL_SCANCODE_DOWN, Key::Down},
    {SDL_SCANCODE_LEFT, Key::Left},
    {SDL_SCANCODE_RIGHT, Key::Right},
    {SDL_SCANCODE_HOME, Key::Home},
    {SDL_SCANCODE_END, Key::End},
    {SDL_SCANCODE_BACKSPACE, Key::Backspace},
    {SDL_SCANCODE_DELETE, Key::Delete},
    {SDL_SCANCODE_F1, Key::F1},
    {SDL_SCANCODE_F2, Key::F2},
    {SDL_SCANCODE_F3, Key::F3},
    {SDL_SCANCODE_F4, Key::F4},
    {SDL_SCANCODE_F5, Key::F5},
    {SDL_SCANCODE_F6, Key::F6},
    {SDL_SCANCODE_F7, Key::F7},
    {SDL_SCANCODE_F8, Key::F8},
    {SDL_SCANCODE_F9, Key::F9},
    {SDL_SCANCODE_F10, Key::F10},
    {SDL_SCANCODE_F11, Key::F11},
    {SDL_SCANCODE_F12, Key::F12},
}};

// Characters that does also insert text
auto specialCharactersMap = std::array<std::pair<int, KeyEvent>, 3>{{
    {SDL_SCANCODE_RETURN, KeyEvent{Key::Return, "\n"}},
    {SDL_SCANCODE_TAB, KeyEvent{Key::Return, "\t"}},
    {SDL_SCANCODE_SPACE, KeyEvent{Key::Return, " "}},
}};

KeyEvent scancodeToMeditKey(SDL_KeyboardEvent event) {
    // Text input is handled separately by matgui

    auto scancode = event.keysym.scancode;

    for (auto pair : keyMap) {
        if (pair.first == scancode) {
            return KeyEvent{pair.second};
        }
    }

    for (auto pair : specialCharactersMap) {
        if (pair.first == scancode) {
            return pair.second;
        }
    }

    auto sym = event.keysym.sym;

    if (sym < 255 && std::isalpha(sym)) {
        return KeyEvent{Key::KeyCombination, std::toupper(sym)};
    }

    return Key::Unknown;
}

bool shouldIgnoreTextInput(char c) {
    switch (c) {
    case '\n':
    case ' ':
        return true;
    default:
        return false;
    }
}

Modifiers modifiers(bool ctrl, bool alt) {
    return static_cast<Modifiers>(
        static_cast<int>(ctrl ? Modifiers::Ctrl : Modifiers{}) |
        static_cast<int>(alt ? Modifiers::Alt : Modifiers{}));
}

} // namespace

struct GuiScreen::Buffer {
    std::vector<FString> lines;
    CursorStyle cursorStyle = CursorStyle::Block;
    size_t width = 0;
    size_t height = 0;
    Position cursorPos;

    sdl::Window window;
    sdl::Renderer renderer;
    matscreen::MatrixScreen screen;

    struct Style {
        sdl::Color fg;
        sdl::Color bg;
    };

    std::vector<Style> styles;

    Buffer(int width, int height)
        : window{"medit",
                 SDL_WINDOWPOS_CENTERED,
                 SDL_WINDOWPOS_CENTERED,
                 width * 20,
                 height * 20,
                 SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN},
          renderer{window, SDL_RENDERER_ACCELERATED, SDL_RENDERER_PRESENTVSYNC},
          screen{width, height, "data/UbuntuMono-Regular.ttf"} {}

    // Save data to be drawn
    void draw(size_t x, size_t y, const FString &str) {
        if (y >= lines.size()) {
            return;
        }

        auto &line = lines.at(y);
        for (size_t i = 0; i < str.size() && i + x < line.size(); ++i) {
            line.at(i + x) = str.at(i);
        }
    }

    sdl::Dims resizePixels(int width,
                           int height,
                           bool shouldUpdateWindow = true) {
        auto dims = sdl::Dims{width / screen.cache.charWidth,
                              height / screen.cache.charHeight};

        resize(dims.w, dims.h, shouldUpdateWindow);

        return dims;
    }

    void resize(int width, int height, bool shouldUpdateWindow = true) {
        if (width == this->width && height == this->height) {
            return;
        }
        lines.resize(height);

        for (auto &line : lines) {
            line.resize(width, ' ');
        }

        this->width = width;
        this->height = height;

        if (shouldUpdateWindow) {
            window.size(width * screen.cache.charWidth,
                        height * screen.cache.charHeight);
        }

        screen.resize(width, height);
    }

    void fill(FChar color) {
        for (auto &line : lines) {
            for (auto &c : line) {
                c = color;
            }
        }
    }

    void renderLine(size_t y, const FString &str) {
        for (size_t x = 0; x < str.size(); ++x) {
            auto c = str.at(x);
            auto &s = screen.canvas.at(x, y);

            // Todo: Check what needs to be updated in some smart way
            s.texture =
                screen.cache.getCharacter(renderer, std::string_view{c.c});

            auto style = [&]() -> Style & {
                if (c.f < styles.size()) {
                    return styles.at(c.f);
                }
                return styles.front();
            }();

            s.bg = style.bg;
            s.fg = style.fg;
        }
    }

    // Update the screen
    void refresh() {
        for (size_t y = 0; y < lines.size(); ++y) {
            renderLine(y, lines.at(y));
        }

        screen.render(renderer);

        renderer.drawColor(sdl::White);

        auto cellWidth = screen.cache.charWidth;
        auto cellHeight = screen.cache.charHeight;

        switch (cursorStyle) {
        case CursorStyle::Beam:
            renderer.fillRect(
                sdl::Rect{static_cast<int>(cellWidth * cursorPos.x()),
                          static_cast<int>(cellHeight * cursorPos.y()),
                          1,
                          static_cast<int>(cellHeight)});
            break;
        default:
            renderer.fillRect(
                sdl::Rect{static_cast<int>(cellWidth * cursorPos.x()),
                          static_cast<int>(cellHeight * cursorPos.y()),
                          static_cast<int>(cellWidth),
                          static_cast<int>(cellHeight)});
            break;
        }

        renderer.present();
    }

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
            {fg.r(), fg.g(), fg.b(), 255},
            {bg.r(), bg.g(), bg.b(), 255},
        };

        return index;
    }
};

void GuiScreen::draw(size_t x, size_t y, const FString &str) {
    _buffer->draw(x, y, str);
}

void GuiScreen::refresh() {
    _buffer->refresh();
}

void GuiScreen::clear() {
    _buffer->fill({});
}

void GuiScreen::cursor(size_t x, size_t y) {
    _buffer->cursorPos.x(x);
    _buffer->cursorPos.y(y);
}

GuiScreen::GuiScreen() : _buffer(std::make_unique<Buffer>(_width, _height)) {
    _buffer->resize(_width, _height);

    sdl::startTextInput();
}

GuiScreen::~GuiScreen() noexcept {
    _buffer.reset();
};

Modifiers getModState() {
    auto modState = sdl::modState();

    bool ctrl = modState & (KMOD_LCTRL | KMOD_RCTRL);
    bool alt = modState & (KMOD_LALT | KMOD_RALT);

    return static_cast<Modifiers>(
        static_cast<int>(ctrl ? Modifiers::Ctrl : Modifiers::None) |
        static_cast<int>(alt ? Modifiers::Alt : Modifiers::None));
}

KeyEvent GuiScreen::getInput() {
    auto sdlEvent = sdl::waitEvent();

    switch (sdlEvent.type) {
    case SDL_QUIT:
        return {Key::Quit};
        break;
    case SDL_KEYDOWN: {
        auto keyEvent = scancodeToMeditKey(sdlEvent.key);

        if (keyEvent.key == Key::Unknown) {
            return keyEvent;
        }

        keyEvent.modifiers = getModState();

        // This is to prevent text input to be registered twice (as text and as
        // keydown)
        if (keyEvent.key == Key::KeyCombination &&
            keyEvent.modifiers == Modifiers::None) {
            return {Key::Unknown};
        }

        return keyEvent;

        break;
    }

    case SDL_TEXTINPUT: {
        auto text = sdlEvent.text;

        auto ch = Utf8Char{text.text};

        if (shouldIgnoreTextInput(ch[0])) {
            return KeyEvent{Key::Unknown};
        }

        return KeyEvent{Key::Text, ch};
    } break;

    case SDL_WINDOWEVENT:
        switch (sdlEvent.window.event) {
        case SDL_WINDOWEVENT_RESIZED: {
            auto dims = _buffer->resizePixels(
                sdlEvent.window.data1, sdlEvent.window.data2, false);
            _width = dims.w;
            _height = dims.h;
            return KeyEvent{Key::Resize};
            break;
        }
        default:
            return Key::Unknown;
            break;
        }
        break;
    }

    return KeyEvent{Key::Unknown};
}

size_t GuiScreen::x() const {
    return 0;
}

size_t GuiScreen::y() const {
    return 0;
}

size_t GuiScreen::width() const {
    return _width;
}

size_t GuiScreen::height() const {
    return _height;
}

void GuiScreen::cursorStyle(CursorStyle style) {
    _buffer->cursorStyle = style;
    return;
}

size_t GuiScreen::addStyle(const Color &fg, const Color &bg, size_t index) {
    return _buffer->addStyle(fg, bg, index);
}
