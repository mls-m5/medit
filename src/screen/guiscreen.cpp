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

auto keyMap = std::array<std::pair<int, Key>, 3>{{
    {Unknown, Key::Unknown},
    {Escape, Key::Escape},
    {Left, Key::Left},
}};

Key matguiToMeditKey(int scanCode, int symbol) {
    // This probably does not work as expected
    if (scanCode >= A && scanCode <= Z) {
        auto s = std::string{};
        return Key::Text;
    }

    for (auto pair : keyMap) {
        if (pair.first == scanCode) {
            return pair.second;
        }
    }

    return Key::Unknown;
}

} // namespace

struct GuiScreen::Buffer {
    std::vector<FString> lines;
    std::vector<matgui::Paint> styles;
    std::map<Utf8Char, matgui::FontView> characters;
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
            auto &bg = styles.at(c.f);
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

        cursorPaint.drawRect((cellWidth)*cursorPos.x(),
                             cellHeight * cursorPos.y(),
                             cellWidth,
                             cellHeight);
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

    _window.frameUpdate.connect([this](double f) { _buffer->refresh(); });
}

GuiScreen::~GuiScreen() noexcept {
    matgui::Application::quit();
    _guiThread.join();
};

KeyEvent GuiScreen::getInput() {
    _inputAvailableMutex.lock();
    if (!_isRunning) {
        _isRunning = true;
        _guiThread = std::thread{[this] { _application.mainLoop(); }};

        _window.keyDown.connect([this](matgui::View::KeyArgument arg) {
            _inputQueue.push_back({
                {},
                {},
                Modifiers::None,
                true,
            });
            _inputAvailableMutex.try_lock();
            _inputAvailableMutex.unlock();
        });
    }

    _inputAvailableMutex.lock();
    auto event = _inputQueue.front();

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

void GuiScreen::cursorStyle(CursorStyle) {
    return;
}

size_t GuiScreen::addStyle(const Color &fg, const Color &bg, size_t index) {
    return _buffer->addStyle(fg, bg, index);
}
