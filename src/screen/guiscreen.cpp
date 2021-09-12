#include "guiscreen.h"
#include "files/fontlocator.h"
#include "matgui/fontview.h"
#include "matgui/keys.h"
#include "matgui/keyutils.h"
#include "matgui/paint.h"
#include "syntax/color.h"
#include "text/position.h"
#include <array>

namespace {

size_t testX = 0;

using namespace matgui::Keys;

auto keyMap = std::array<std::pair<int, Key>, 22>{{
    {Unknown, Key::Unknown}, {Escape, Key::Escape}, {Up, Key::Up},
    {Down, Key::Down},       {Left, Key::Left},     {Right, Key::Right},
    {Home, Key::Home},       {End, Key::End},       {Backspace, Key::Backspace},
    {Delete, Key::Delete},   {F1, Key::F1},         {F2, Key::F2},
    {F3, Key::F3},           {F4, Key::F4},         {F5, Key::F5},
    {F6, Key::F6},           {F7, Key::F7},         {F8, Key::F8},
    {F9, Key::F9},           {F10, Key::F10},       {F11, Key::F11},
    {F12, Key::F12},
}};

// Characters that does also insert text
auto specialCharactersMap = std::array<std::pair<int, KeyEvent>, 3>{{
    {Return, KeyEvent{Key::Return, '\n'}},
    {Tab, KeyEvent{Key::Return, '\t'}},
    {Space, KeyEvent{Key::Return, ' '}},
}};

KeyEvent matguiToMeditKey(int scanCode) {
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
    std::vector<matgui::Paint> styles;
    std::map<Utf8Char, matgui::FontView> characters;
    CursorStyle cursorStyle = CursorStyle::Block;
    size_t width = 0;
    size_t height = 0;
    double cellWidth = 8;
    double cellHeight = 16;
    matgui::Font font = {findFont("UbuntuMono-R").string(), 10};

    Position cursorPos;

    matgui::Paint cursorPaint = {
        {1, 1, 1},
        {1, 1, 1},
        {},
    };

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
    }

    void fill(FChar color) {
        for (auto &line : lines) {
            for (auto &c : line) {
                c = color;
            }
        }
    }

    matgui::FontView &getFontView(Utf8Char c) {
        if (auto f = characters.find(c); f != characters.end()) {
            return f->second;
        }

        auto &f = characters[c];
        f.font(font);
        f.text(c);
        return f;
    }

    void renderLine(size_t y, const FString &str) {
        for (size_t x = 0; x < str.size(); ++x) {
            auto c = str.at(x);
            auto &f = getFontView(c.c);
            auto &bg = [&]() -> matgui::Paint & {
                if (c.f < styles.size()) {
                    return styles.at(c.f);
                }
                return styles.front();
            }();
            bg.drawRect(x * cellWidth, y * cellHeight, cellWidth, cellHeight);
            f.draw(x * cellWidth + cellWidth / 2 * 0,
                   y * cellHeight + cellHeight * .6);
        }
    }

    // Update the screen
    void refresh() {
        for (size_t y = 0; y < lines.size(); ++y) {
            renderLine(y, lines.at(y));
        }

        switch (cursorStyle) {
        case CursorStyle::Beam:
            cursorPaint.drawRect(cellWidth * cursorPos.x(),
                                 cellHeight * cursorPos.y(),
                                 1,
                                 cellHeight);
            break;
        default:
            cursorPaint.drawRect(cellWidth * cursorPos.x(),
                                 cellHeight * cursorPos.y(),
                                 cellWidth,
                                 cellHeight);
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

        style.fill.color(bg.r() / 255., bg.g() / 255., bg.b() / 255.);
        style.line.color(0, 0, 0, 0);

        return index;
    }
};

void GuiScreen::draw(size_t x, size_t y, const FString &str) {
    _buffer->draw(x, y, str);
}

void GuiScreen::refresh() {
    _window.invalidate();
}

void GuiScreen::clear() {
    _buffer->fill({});
}

void GuiScreen::cursor(size_t x, size_t y) {
    _buffer->cursorPos.x(x);
    _buffer->cursorPos.y(y);
}

GuiScreen::GuiScreen()
    : _buffer(std::make_unique<Buffer>()), _application(0, nullptr),
      _window("matedit",
              constWidth * _buffer->cellWidth,
              constHeight * _buffer->cellHeight) {
    _buffer->resize(constWidth, constHeight);
    matgui::beginTextEntry();

    _window.frameUpdate.connect([this](double f) { _buffer->refresh(); });
}

GuiScreen::~GuiScreen() noexcept {
    matgui::Application::quit();
    _guiThread.join();
};

KeyEvent GuiScreen::getInput() {
    if (!_isRunning) {
        _inputAvailableMutex.lock();
        _isRunning = true;
        _guiThread = std::thread{[this] { _application.mainLoop(); }};

        _window.textInput.connect([this](std::string text) {
            auto l = std::scoped_lock{_queueLock};

            if (text == " " || text == "\t") {
                // Avoid double handling
                return;
            }

            _inputQueue.emplace_back(Key::Text,
                                     text.c_str(),
                                     modifiers(_ctrlState, _altState),
                                     true);
            _inputAvailableMutex.try_lock();
            _inputAvailableMutex.unlock();
        });

        _window.keyDown.connect([this](matgui::View::KeyArgument arg) {
            auto l = std::scoped_lock{_queueLock};
            // Handle special keys (not text)
            if (auto key = matguiToMeditKey(arg.scanCode);
                key != Key::Unknown) {
                key.modifiers = modifiers(_ctrlState, _altState);
                _inputQueue.emplace_back(key);
                _inputAvailableMutex.try_lock();
                _inputAvailableMutex.unlock();
            }
            else {
                if (arg.scanCode == matgui::Keys::CtrlLeft) {
                    _ctrlState = true;
                }
                else if (arg.scanCode == matgui::Keys::AltLeft) {
                    _altState = true;
                }
                else if (_ctrlState) {
                    // For some reason when holding ctrl. Text input does not
                    // work

                    if (arg.symbol >= 'A' || arg.symbol <= 'Z') {
                        arg.symbol -= ('a' - 'A');

                        auto event = KeyEvent{Key::KeyCombination,
                                              arg.symbol,
                                              modifiers(_ctrlState, _altState)};
                        _inputQueue.emplace_back(event);
                        _inputAvailableMutex.try_lock();
                        _inputAvailableMutex.unlock();
                    }
                }
            }
        });

        _window.keyUp.connect([this](matgui::View::KeyArgument arg) {
            if (arg.scanCode == matgui::Keys::CtrlLeft) {
                _ctrlState = false;
            }
            else if (arg.scanCode == matgui::Keys::AltLeft) {
                _altState = false;
            }
        });
    }

    _inputAvailableMutex.try_lock();
    {
        auto l = std::unique_lock{_queueLock};
        if (_inputQueue.empty()) {
            l.unlock();
            _inputAvailableMutex.lock(); // Lock untill there is some key
            l.lock();
        }
    }
    auto event = _inputQueue.front();
    _inputQueue.erase(_inputQueue.begin());

    ++testX;

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
