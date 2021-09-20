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
    {SDLK_UNKNOWN, Key::Unknown},
    {SDLK_ESCAPE, Key::Escape},
    {SDLK_UP, Key::Up},
    {SDLK_DOWN, Key::Down},
    {SDLK_LEFT, Key::Left},
    {SDLK_RIGHT, Key::Right},
    {SDLK_HOME, Key::Home},
    {SDLK_END, Key::End},
    {SDLK_BACKSPACE, Key::Backspace},
    {SDLK_DELETE, Key::Delete},
    {SDLK_F1, Key::F1},
    {SDLK_F2, Key::F2},
    {SDLK_F3, Key::F3},
    {SDLK_F4, Key::F4},
    {SDLK_F5, Key::F5},
    {SDLK_F6, Key::F6},
    {SDLK_F7, Key::F7},
    {SDLK_F8, Key::F8},
    {SDLK_F9, Key::F9},
    {SDLK_F10, Key::F10},
    {SDLK_F11, Key::F11},
    {SDLK_F12, Key::F12},
}};

// Characters that does also insert text
auto specialCharactersMap = std::array<std::pair<int, KeyEvent>, 3>{{
    {SDLK_RETURN, KeyEvent{Key::Return, '\n'}},
    {SDLK_TAB, KeyEvent{Key::Return, '\t'}},
    {SDLK_SPACE, KeyEvent{Key::Return, ' '}},
}};

KeyEvent scancodeToMeditKey(int scanCode) {
    // Text input is handled separately by matgui

    for (auto pair : keyMap) {
        if (pair.first == scanCode) {
            return KeyEvent{pair.second};
        }
    }

    for (auto pair : specialCharactersMap) {
        if (pair.first == scanCode) {
            return pair.second;
        }
    }

    return Key::Unknown;
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
    double cellWidth = 8;
    double cellHeight = 16;
    Position cursorPos;

    sdl::Window window;
    sdl::Renderer renderer;
    matscreen::MatrixScreen screen;
    std::vector<sdl::Color> styles;

    Buffer()
        : window{"medit",
                 300,
                 200,
                 SDL_WINDOWPOS_CENTERED,
                 SDL_WINDOWPOS_CENTERED,
                 SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN},
          renderer{
              window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC},
          screen{300, 200} {}

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

    void resize(size_t width, size_t height) {
        if (width == this->width && height == this->height) {
            return;
        }
        lines.resize(height);

        for (auto &line : lines) {
            line.resize(width, ' ');
        }

        this->width = width;
        this->height = height;

        window.size(width * screen.cache.charWidth,
                    height * screen.cache.charHeight);
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
            auto s = screen.canvas.at(x, y);

            // Todo: Check what needs to be updated in some smart way
            s.texture =
                screen.cache.getCharacter(renderer, std::string_view{c.c});

            s.bg = [&]() -> sdl::Color & {
                if (c.f < styles.size()) {
                    return styles.at(c.f);
                }
                return styles.front();
            }();
        }
    }

    // Update the screen
    void refresh() {
        for (size_t y = 0; y < lines.size(); ++y) {
            renderLine(y, lines.at(y));
        }

        screen.render(renderer);

        renderer.drawColor(sdl::White);
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

        style = {bg.r(), bg.g(), bg.b()};

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

GuiScreen::GuiScreen() : _buffer(std::make_unique<Buffer>()) {
    _buffer->resize(constWidth, constHeight);

    sdl::startTextInput();
}

GuiScreen::~GuiScreen() noexcept {
    _buffer.reset();
    _guiThread.join();
};

KeyEvent GuiScreen::getInput() {
    //    if (!_isRunning) {
    //        _inputAvailableMutex.lock();
    //        _isRunning = true;
    //        _guiThread = std::thread{[this] { _application.mainLoop(); }};

    //        _window.textInput.connect([this](std::string text) {
    //            auto l = std::scoped_lock{_queueLock};

    //            if (text == " " || text == "\t") {
    //                // Avoid double handling
    //                return;
    //            }

    //            _inputQueue.emplace_back(Key::Text,
    //                                     text.c_str(),
    //                                     modifiers(_ctrlState, _altState),
    //                                     true);
    //            _inputAvailableMutex.try_lock();
    //            _inputAvailableMutex.unlock();
    //        });

    //        _window.keyDown.connect([this](matgui::View::KeyArgument arg) {
    //            auto l = std::scoped_lock{_queueLock};
    //            // Handle special keys (not text)
    //            if (auto key = scancodeToMeditKey(arg.scanCode);
    //                key != Key::Unknown) {
    //                key.modifiers = modifiers(_ctrlState, _altState);
    //                _inputQueue.emplace_back(key);
    //                _inputAvailableMutex.try_lock();
    //                _inputAvailableMutex.unlock();
    //            }
    //            else {
    //                if (arg.scanCode == matgui::Keys::CtrlLeft) {
    //                    _ctrlState = true;
    //                }
    //                else if (arg.scanCode == matgui::Keys::AltLeft) {
    //                    _altState = true;
    //                }
    //                else if (_ctrlState) {
    //                    // For some reason when holding ctrl. Text input does
    //                    not
    //                    // work

    //                    if (arg.symbol >= 'A' || arg.symbol <= 'Z') {
    //                        arg.symbol -= ('a' - 'A');

    //                        auto event = KeyEvent{Key::KeyCombination,
    //                                              arg.symbol,
    //                                              modifiers(_ctrlState,
    //                                              _altState)};
    //                        _inputQueue.emplace_back(event);
    //                        _inputAvailableMutex.try_lock();
    //                        _inputAvailableMutex.unlock();
    //                    }
    //                }
    //            }
    //        });

    //        _window.keyUp.connect([this](matgui::View::KeyArgument arg) {
    //            if (arg.scanCode == matgui::Keys::CtrlLeft) {
    //                _ctrlState = false;
    //            }
    //            else if (arg.scanCode == matgui::Keys::AltLeft) {
    //                _altState = false;
    //            }
    //        });
    //    }

    //    _inputAvailableMutex.try_lock();
    //    {
    //        auto l = std::unique_lock{_queueLock};
    //        if (_inputQueue.empty()) {
    //            l.unlock();
    //            _inputAvailableMutex.lock(); // Lock untill there is some key
    //            l.lock();
    //        }
    //    }
    //    auto event = _inputQueue.front();
    //    _inputQueue.erase(_inputQueue.begin());

    //    ++testX;

    auto sdlEvent = sdl::waitEvent();

    switch (sdlEvent.type) {
        // TODO: continue here
    }

    return event;
}

size_t GuiScreen::x() const {
    return 0;
}

size_t GuiScreen::y() const {
    return 0;
}

size_t GuiScreen::width() const {
    return constWidth;
}

size_t GuiScreen::height() const {
    return constHeight;
}

void GuiScreen::cursorStyle(CursorStyle style) {
    _buffer->cursorStyle = style;
    return;
}

size_t GuiScreen::addStyle(const Color &fg, const Color &bg, size_t index) {
    return _buffer->addStyle(fg, bg, index);
}
