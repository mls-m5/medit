#include "guiscreen.h"
#include "core/profiler.h"
#include "core/threadname.h"
#include "sdlpp/rect.hpp"
#include "sdlpp/surface.hpp"
#include <algorithm>
#include <mutex>
#include <thread>

#ifndef __EMSCRIPTEN__

#include "SDL2/SDL_keyboard.h"
#include "core/os.h"
#include "core/threadvalidation.h"
#include "files/fontlocator.h"
#include "matrixscreen.h"
#include "screen/ipixelsource.h"
#include "screen/iscreen.h"
#include "sdlpp/events.hpp"
#include "sdlpp/keyboard.hpp"
#include "sdlpp/render.hpp"
#include "sdlpp/window.hpp"
#include "syntax/color.h"
#include "syntax/palette.h"
#include "text/fstringview.h"
#include "text/position.h"
#include <array>
#include <memory>
#include <mutex>
#include <thread>

namespace {

auto keyMap = std::array<std::pair<int, Key>, 24>{{
    {SDLK_UNKNOWN, Key::Unknown},
    {SDLK_ESCAPE, Key::Escape},
    {SDLK_UP, Key::Up},
    {SDLK_DOWN, Key::Down},
    {SDLK_LEFT, Key::Left},
    {SDLK_RIGHT, Key::Right},
    {SDLK_HOME, Key::Home},
    {SDLK_END, Key::End},
    {SDLK_PAGEUP, Key::PageUp},
    {SDLK_PAGEDOWN, Key::PageDown},
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
    {SDLK_RETURN, KeyEvent{Key::Text, "\n"}},
    {SDLK_TAB, KeyEvent{Key::Tab, "\t"}},
    {SDLK_SPACE, KeyEvent{Key::Space, " "}},
}};

KeyEvent keycodeToMeditKey(SDL_KeyboardEvent event) {
    // Text input is handled separately by matgui

    auto sym = event.keysym.sym;

    for (auto pair : keyMap) {
        if (pair.first == sym) {
            return KeyEvent{pair.second};
        }
    }

    for (auto pair : specialCharactersMap) {
        if (pair.first == sym) {
            return pair.second;
        }
    }

    if (sym < 255 && (std::isalnum(sym) || isspace(sym))) {
        return KeyEvent{Key::KeyCombination,
                        static_cast<char>(std::toupper(sym))};
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

Modifiers getModState() {
    auto modState = sdl::modState();

    bool ctrl = modState & (KMOD_LCTRL | KMOD_RCTRL);
    bool alt = modState & (KMOD_LALT | KMOD_RALT);

    return static_cast<Modifiers>(
        static_cast<int>(ctrl ? Modifiers::Ctrl : Modifiers::None) |
        static_cast<int>(alt ? Modifiers::Alt : Modifiers::None));
}
} // namespace

/// The rendering and gui is running on one thread and assumes the application
/// calls from one single thread
struct GuiScreen : public virtual IGuiScreen, public virtual IPixelSource {
    std::vector<FString> lines;
    std::vector<FString> shownLines;
    std::mutex refreshMutex;
    CursorStyle _cursorStyle = CursorStyle::Block;
    size_t _width = 0;
    size_t _height = 0;
    size_t pixelWidth = 0;
    size_t pixelHeight = 0;
    Position _cursorPos;

    sdl::Window window;
    sdl::Renderer renderer;
    matscreen::MatrixScreen screen;

    IGuiScreen::CallbackT _callback;

    struct Style {
        sdl::Color fg = sdl::White;
        sdl::Color bg;
    };

    std::vector<Style> _styles;
    std::thread _thread;

    bool _isRunning = true;
    bool _shouldRefresh = true;

    ThreadValidation _tv{"gui screen"};

    /// Mutex used in creation to make sure some functions used for setup is not
    /// accessed to early
    std::mutex _createMutex;

    Palette _palette;

    GuiScreen(int width, int height, int fontSize)
        : window{"medit",
                 SDL_WINDOWPOS_CENTERED,
                 SDL_WINDOWPOS_CENTERED,
                 width * 8,
                 height * 8,
                 SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN}
        , renderer{window,
                   SDL_RENDERER_ACCELERATED,
                   0 * SDL_RENDERER_PRESENTVSYNC}
        , screen{width, height, fontPath(), fontSize} {
        // std::condition_variable cv;

        sdl::startTextInput();
        _styles.resize(16);
        resizeInternal(width, height);

        auto lock = std::unique_lock{_createMutex};

        _thread = std::thread([this] {
            _tv.reset();
            setThreadName("gui screen");
            loop();
        });
    }

    ~GuiScreen() {
        // Just make the guiloop run and exit, and class is
        // specified to avoid virtual function call in destructor
        GuiScreen::refresh();
        stop();
        _thread.join();
    }

    void stop() {
        _isRunning = false;
    }

    // Save data to be drawn
    void draw(size_t x, size_t y, FStringView str) override {
        if (y >= lines.size()) {
            return;
        }

        auto &line = lines.at(y);
        for (size_t i = 0; i < str.size() && i + x < line.size(); ++i) {
            line.at(i + x) = str.at(i);
        }
    }

    void title(std::string title) override {
        window.title(title.c_str());
    }

    std::string fontPath() {
        {
            auto path = findFont("SourceCodePro-Regular");
            if (!path.empty()) {
                return path.string();
            }
        }
        {
            auto path = findFont("UbuntuMono-Regular");
            if (!path.empty()) {
                return path.string();
            }
        }
        return findFont("UbuntuMono-R"); // Try to find system font
    }

    sdl::Dims resizePixels(int width,
                           int height,
                           bool shouldUpdateWindow = true) {
        auto dims = sdl::Dims{width / screen.cache.charWidth,
                              height / screen.cache.charHeight};

        resizeInternal(dims.w, dims.h, shouldUpdateWindow);

        pixelWidth = width;
        pixelHeight = height;

        return dims;
    }

    void resize(int width, int height) override {
        resizeInternal(width, height, true);
    }

    void resizeInternal(int width, int height, bool shouldUpdateWindow = true) {
        if (width == this->_width && height == this->_height) {
            return;
        }

        if (width <= 0 || height <= 0) {
            return;
        }

        lines.resize(height);

        for (auto &line : lines) {
            line.resize(width, ' ');
        }

        {
            auto l = std::lock_guard{refreshMutex};
            shownLines.resize(height);
            for (auto &line : shownLines) {
                line.resize(width, ' ');
            }
        }

        this->_width = width;
        this->_height = height;

        if (shouldUpdateWindow) {
            window.size(width * screen.cache.charWidth,
                        height * screen.cache.charHeight);
            pixelWidth = width * screen.cache.charWidth;
            pixelHeight = height * screen.cache.charHeight;
        }

        screen.resize(width, height);
    }

    void fontSize(int size) override {
        auto l = std::lock_guard{
            refreshMutex}; /// Make sure the lines is not currently being drawn
        screen.resizeFont(size);
        pixelWidth = screen.cache.charWidth * _width;
        pixelHeight = screen.cache.charHeight * _height;
    }

    void fill(FChar color) {
        for (auto &line : lines) {
            for (auto &c : line) {
                c = color;
            }
        }
    }

    void renderLine(size_t y, FStringView str) {
        for (size_t x = 0; x < str.size() && x < screen.canvas.width; ++x) {
            auto c = str.at(x);

            if (!static_cast<uint32_t>(c.c)) {
                continue;
            }
            auto &s = screen.canvas.at(x, y);

            // Todo: Check what needs to be updated in some smart way
            s.texture =
                screen.cache.getCharacter(renderer, std::string_view{c.c});

            auto style = [&]() -> Style & {
                if (c.f < _styles.size()) {
                    return _styles.at(c.f);
                }
                return _styles.front();
            }();

            s.bg = style.bg;
            s.fg = style.fg;
        }
    }

    // Make sure that the bottom line aligns with the window border
    void drawBottomLine(sdl::RendererView renderer) {
        auto d = ProfileDuration{};
        screen.render(renderer,
                      0,
                      pixelHeight - screen.cache.charHeight,
                      {0, screen.canvas.height - 1, screen.canvas.width, 1});
    }

    // Copy lines to be refreshed, may be called from the applications thread
    // and not the gui thread
    void copyLines() {
        auto l = std::lock_guard{
            refreshMutex}; /// Make sure the lines is not currently being drawn
        std::copy(lines.begin(), lines.end(), shownLines.begin());
        _shouldRefresh = true;
        auto event = SDL_Event{};
        event.type = SDL_USEREVENT;
        sdl::pushEvent(event);
    }

    // Update the screen
    void updateScreen() {
        auto duration = ProfileDuration{};

        auto l = std::lock_guard{refreshMutex};
        _tv();
        {
            auto d = ProfileDuration{"Draw"};

            {
                auto d = ProfileDuration{"Render Lines"};
                for (size_t y = 0; y < shownLines.size(); ++y) {
                    renderLine(y, shownLines.at(y));
                }
            }

            renderer.drawColor(_styles.front().bg);
            renderer.fillRect();

            auto rect =
                sdl::Rect{0, 0, screen.canvas.width, screen.canvas.height - 1};
            {
                auto d = ProfileDuration{"RenderScreen"};
                screen.render(renderer, 0, 0, rect);
            }
            drawBottomLine(renderer);

            renderer.drawColor(sdl::White);

            auto cellWidth = screen.cache.charWidth;
            auto cellHeight = screen.cache.charHeight;

            switch (_cursorStyle) {
            case CursorStyle::Beam:
                renderer.fillRect(
                    sdl::Rect{static_cast<int>(cellWidth * _cursorPos.x()),
                              static_cast<int>(cellHeight * _cursorPos.y()),
                              1,
                              static_cast<int>(cellHeight)});
                break;
            case CursorStyle::Hidden:
                break;
            default:
                screen.renderCursor(
                    renderer, rect, _cursorPos.x(), _cursorPos.y());
                break;
            }
        }

        {
            auto d = ProfileDuration{"Present"};
            renderer.present();
        }
    }

    size_t addStyle(const Color &fg, const Color &bg, size_t index) override {
        if (index != std::numeric_limits<size_t>::max() &&
            index < _styles.size()) {
        }
        else {
            index = _styles.size();
            _styles.emplace_back();
        }

        auto &style = _styles.at(index);

        style = {
            {fg.r(), fg.g(), fg.b(), 255},
            {bg.r(), bg.g(), bg.b(), 255},
        };

        return index;
    }

    void subscribe(IGuiScreen::CallbackT f) override {
        auto lock = std::unique_lock{_createMutex};
        _callback = f;
    }

    void unsubscribe() override {
        _callback = {};
    }

    void loop() {
        _tv();
        // TODO: Refactor this to use sdls wating functions
        using namespace std::literals;
        for (; _isRunning;) {
            auto list = IScreen::EventListT{};

            if (auto e = getInput(true);
                !std::holds_alternative<NullEvent>(e)) {
                list.push_back(std::move(e));
            }
            for (Event e;
                 e = getInput(false), !std::holds_alternative<NullEvent>(e);) {
                list.push_back(std::move(e));
            }

            if (!list.empty() && _callback) {
                _callback(std::move(list));
            }

            if (_shouldRefresh) {
                updateScreen(); // TODO: Only refresh when changes
                _shouldRefresh = false;
            }
        }
    }

    Event getInput(bool wait) {
        _tv();
        auto e = [wait] {
            if (getOs() == Os::Emscripten) {
                return sdl::pollEvent();
            }
            else {
                if (wait) {
                    return sdl::waitEventTimeout(10000);
                }
                else {
                    return sdl::pollEvent();
                }
            }
        }();

        auto duration = ProfileDuration{};

        if (!e) {
            return NullEvent{};
        }

        auto sdlEvent = *e;

        switch (sdlEvent.type) {
        case SDL_QUIT:
            return {Key::Quit};
            break;
        case SDL_KEYDOWN: {
            auto keyEvent = keycodeToMeditKey(sdlEvent.key);

            if (keyEvent.key == Key::Unknown) {
                return NullEvent{};
            }

            if (sdlEvent.key.keysym.mod == 64) {
                auto sym = sdlEvent.key.keysym.sym;

                if (sym == 'v') {
                    if (SDL_HasClipboardText()) {
                        return PasteEvent{SDL_GetClipboardText()};
                    }
                    return NullEvent{};
                }
            }

            keyEvent.modifiers = getModState();

            // Sometimes the alt-tab (switching windows) goes through to the
            // window. This is to ignore that event
            // This the logic needs to be more advanced than this though
            //            if (keyEvent.modifiers == Modifiers::Alt &&
            //                keyEvent.key == Key::Tab) {
            //                return NullEvent{};
            //            }

            if (keyEvent.modifiers != Modifiers::None &&
                (keyEvent.key == Key::Space || keyEvent.symbol == "\n" ||
                 keyEvent.key == Key::Tab || keyEvent.key == Key::Backspace)) {

                if (keyEvent.key == Key::Backspace) {
                    keyEvent.symbol = '\b';
                }

                keyEvent.key = Key::KeyCombination;
            }

            // This is to prevent text input to be registered twice (as text and
            // as keydown)
            if (keyEvent.key == Key::KeyCombination &&
                keyEvent.modifiers == Modifiers::None) {
                return NullEvent{};
            }

            return keyEvent;

            break;
        }

        case SDL_TEXTINPUT: {
            auto text = sdlEvent.text;

            auto ch = Utf8Char{text.text};

            if (shouldIgnoreTextInput(ch[0])) {
                return NullEvent{};
            }

            return KeyEvent{Key::Text, ch};
        } break;

        case SDL_WINDOWEVENT:
            switch (sdlEvent.window.event) {
            case SDL_WINDOWEVENT_RESIZED: {
                resizePixels(
                    sdlEvent.window.data1, sdlEvent.window.data2, false);
                auto r = ResizeEvent{};
                r.width = _width;
                r.height = _height;
                return r;
                break;
            }
            default:
                return NullEvent{};
                break;
            }
            break;

        case SDL_MOUSEWHEEL:
            if (sdlEvent.wheel.y > 0) {
                return KeyEvent{Key::PageUp};
            }
            else if (sdlEvent.wheel.y < 0) {
                return KeyEvent{Key::PageDown};
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            return MouseDownEvent{1,
                                  sdlEvent.button.x / screen.cache.charWidth,
                                  sdlEvent.button.y / screen.cache.charHeight};

            break;
        }

        return NullEvent{};
    }

    sdl::Surface readPixels() override {
        using namespace std::chrono_literals;
        while (_shouldRefresh) {
            std::this_thread::sleep_for(1ms);
        }
        auto l = std::lock_guard{refreshMutex};
        auto rect = sdl::Rect{
            0, 0, static_cast<int>(pixelWidth), static_cast<int>(pixelHeight)};
        auto surface = sdl::Surface::create(0,
                                            pixelWidth,
                                            pixelHeight,
                                            32,
                                            0x00FF0000,
                                            0x0000FF00,
                                            0x000000FF,
                                            0xFF000000);

        if (renderer.readPixels(sdl::SurfaceView{surface})) {
            return nullptr;
        }

        return surface;
    }

    std::string clipboardData() override {
        return SDL_GetClipboardText();
    }

    void clipboardData(std::string text) override {
        SDL_SetClipboardText(text.data());
    }

    void clear() override {
        fill({});
    }

    void cursor(size_t x, size_t y) override {
        _cursorPos.x(x);
        _cursorPos.y(y);
    }

    size_t width() const override {
        return static_cast<size_t>(_width);
    }

    size_t height() const override {
        return static_cast<size_t>(_height);
    }

    void cursorStyle(CursorStyle style) override {
        _cursorStyle = style;
    }

    void palette(const Palette &palette) override {
        _palette = palette;
    }

    void refresh() override {
        auto duration = ProfileDuration{};
        if (_palette.isChanged()) {
            _palette.update(*this);
        }
        copyLines();
    }
};

std::unique_ptr<IGuiScreen> createGuiScreen() {
    return std::make_unique<GuiScreen>(85, 40, 14);
}

#endif
