
#include "screen/bufferedscreen.h"
#include "screen/iscreen.h"

struct BufferedScreen::Canvas {
    std::vector<FString> lines;
    size_t width = 0;
    size_t height = 0;
    size_t _cursorX = 0, _cursorY = 0;

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

    void draw(size_t x, size_t y, const FString &str) {
        if (y >= lines.size()) {
            return;
        }

        auto &line = lines.at(y);
        for (size_t i = 0; i < str.size() && i + x < line.size(); ++i) {
            line.at(i + x) = str.at(i);
        }
    }

    void fill(FChar color) {
        for (auto &line : lines) {
            for (auto &c : line) {
                c = color;
            }
        }
    }

    void refresh(IScreen &screen) {
        for (size_t y = 0; y < lines.size(); ++y) {
            screen.draw(0, y, lines.at(y));
        }

        screen.cursor(_cursorX, _cursorY);
    }
};

void BufferedScreen::forceThread() {
    if (_threadId != std::this_thread::get_id()) {
        throw std::runtime_error{
            "trying to run from wrong thread in bufferedscreen"};
    }
}

BufferedScreen::BufferedScreen(IScreen *screen, IInput *input)
    : _backend(screen)
    , _input(input)
    , _canvas(std::make_unique<Canvas>())
    , _threadId(std::this_thread::get_id()) {
    _canvas->resize(_backend->width(), _backend->height());
}

BufferedScreen::~BufferedScreen() = default;

void BufferedScreen::draw(size_t x, size_t y, const FString &str) {
    forceThread();
    _canvas->draw(x, y, str);
}

void BufferedScreen::refresh() {
    forceThread();
    _canvas->resize(_backend->width(), _backend->height());
    _canvas->refresh(*_backend);
    _backend->refresh();
}

void BufferedScreen::clear() {
    forceThread();
    _canvas->fill(' ');
}

void BufferedScreen::cursor(size_t x, size_t y) {
    forceThread();
    _canvas->_cursorX = x;
    _canvas->_cursorY = y;
}

size_t BufferedScreen::x() const {
    return _backend->x();
}

size_t BufferedScreen::y() const {
    return _backend->y();
}

size_t BufferedScreen::width() const {
    return _backend->width();
}

size_t BufferedScreen::height() const {
    return _backend->height();
}

void BufferedScreen::title(std::string title) {
    _backend->title(title);
}

const IPalette &BufferedScreen::palette() const {
    return _backend->palette();
}

IPalette &BufferedScreen::palette() {
    return _backend->palette();
}

size_t BufferedScreen::addStyle(const Color &foreground,
                                const Color &background,
                                size_t index) {
    return _backend->addStyle(foreground, background, index);
}

void BufferedScreen::cursorStyle(CursorStyle style) {
    forceThread();
    _backend->cursorStyle(style);
}

Event BufferedScreen::getInput() {
    auto event = _input->getInput();
    if (std::holds_alternative<KeyEvent>(event);
        std::get<KeyEvent>(event) == Key::Resize) {
        _canvas->resize(_backend->width(), _backend->height());
    }
    return event;
}
