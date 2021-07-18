#include "guiscreen.h"
#include "matgui/keys.h"
#include "matgui/keyutils.h"
#include "matgui/paint.h"
#include "syntax/color.h"
#include <array>

namespace {

auto testPaint =
    matgui::Paint{matgui::ColorStyle{1, 1, 1}, matgui::LineStyle{1, 1, 1}};

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
    size_t width = 0;
    size_t height = 0;
    double cellWith = 10;
    double cellHeight = 20;

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

    void renderLine(size_t y, const FString &str) {
        for (size_t x = 0; x < str.size(); ++x) {
            auto c = str.at(x);
            auto &paint = styles.at(c.f);
            paint.drawRect(x * cellWith, y * cellHeight, cellWith, cellHeight);
        }
    }

    // Update the screen
    void refresh() {
        for (size_t y = 0; y < lines.size(); ++y) {
            renderLine(y, lines.at(y));
        }

        testPaint.drawRect(
            (cellWith) * (3 + testX), cellHeight * 3, cellWith, cellHeight);
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
        style.line.color(fg.r() / 255., fg.g() / 255., fg.b() / 255.);

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

void GuiScreen::cursor(size_t x, size_t y) {}

GuiScreen::GuiScreen()
    : _buffer(std::make_unique<Buffer>()), _application(0, nullptr),
      _window("matedit",
              constWidth * _buffer->cellWith,
              constHeight * _buffer->cellHeight) {
    _buffer->resize(constWidth, constHeight);

    _window.frameUpdate.connect([this](double f) { _buffer->refresh(); });
}

GuiScreen::~GuiScreen() noexcept {
    matgui::Application::quit();
    _guiThread.join();
};

KeyEvent GuiScreen::getInput() {
    _inputMutex.lock();
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
            _inputMutex.try_lock();
            _inputMutex.unlock();
        });
    }

    _inputMutex.lock();
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
