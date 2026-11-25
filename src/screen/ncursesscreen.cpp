#include "core/profiler.h"
#include "core/threadname.h"
#ifndef __EMSCRIPTEN__
#include "ncursesscreen.h"
#include "syntax/color.h"
#include <climits>
#include <clocale>
#include <cwchar>
#include <map>
#include <ncurses.h>

namespace {

struct KeyTranslation {
    Key key;
    Utf8Char text{};
};

//! Keys that starts with ^
const auto keytranslations = std::map<int, KeyTranslation>{
    {KEY_BACKSPACE, {Key::Backspace}},
    {10, {Key::Text, "\n"}},
    //    {KEY_ENTER, {Key::Return, "\n"}},
    {KEY_ENTER, {Key::Text, "\n"}},
    {32, {Key::Space, " "}},
    {KEY_DOWN, {Key::Down}},
    {KEY_UP, {Key::Up}},
    {KEY_LEFT, {Key::Left}},
    {KEY_RIGHT, {Key::Right}},
    {KEY_HOME, {Key::Home}},
    {KEY_END, {Key::End}},
    {KEY_NPAGE, {Key::PageDown}},
    {KEY_PPAGE, {Key::PageUp}},
    {27, {Key::Escape}},
    {KEY_CANCEL, {Key::Cancel}},
    {KEY_DC, {Key::Delete}},
    {KEY_STAB, {Key::Tab}},
    {KEY_F(1), {Key::F1}},
    {KEY_F(2), {Key::F2}},
    {KEY_F(3), {Key::F3}},
    {KEY_F(4), {Key::F4}},
    {KEY_F(5), {Key::F5}},
    {KEY_F(6), {Key::F6}},
    {KEY_F(7), {Key::F7}},
    {KEY_F(8), {Key::F8}},
    {KEY_F(9), {Key::F9}},
    {KEY_F(10), {Key::F10}},
    {KEY_F(11), {Key::F11}},
    {KEY_F(12), {Key::F12}},
};

void reducedPalette() {
    ::init_pair(Palette::standard, COLOR_WHITE, COLOR_BLACK);
    ::init_pair(Palette::selection, COLOR_BLUE, COLOR_WHITE);
    ::init_pair(Palette::statement, COLOR_RED, COLOR_BLACK);
    ::init_pair(Palette::identifier, COLOR_RED, COLOR_BLACK);
    ::init_pair(Palette::comment, COLOR_BLUE, COLOR_BLACK);
    ::init_pair(Palette::lineNumbers, COLOR_BLUE, COLOR_BLACK);
    ::init_pair(Palette::currentLine, COLOR_WHITE, COLOR_BLACK);
    ::init_pair(Palette::string, COLOR_YELLOW, COLOR_BLACK);
    ::init_pair(Palette::type, COLOR_RED, COLOR_BLACK);
    ::init_pair(Palette::error, COLOR_BLACK, COLOR_RED);
}

} // namespace

void NCursesScreen::init() {
    PROFILE_FUNCTION();
    // ncurses must know the terminal's multibyte locale before initscr().
    // Otherwise printw counts UTF-8 bytes as cells and the cursor drifts.
    std::setlocale(LC_CTYPE, "");
    ::initscr();

    _tv.reset();

    if (!has_colors()) {
        _hasColors = false;
    }

    if (auto s = getenv("GJS_PATH");
        s && std::string{s}.find("drop-down") != std::string::npos) {
        // Special case for gnome drop down terminal
        _hasColors = false;
    }

    ::start_color();
    if (_hasColors) {
    }
    else {
        reducedPalette();
    }
    ::raw();
    ::keypad(stdscr, true);
    ::noecho();

    //    mousemask(ALL_MOUSE_EVENTS, NULL);
    mousemask(BUTTON1_PRESSED, NULL);

    ::timeout(10);
}

void NCursesScreen::draw(size_t x, size_t y, FStringView rstr) {
    PROFILE_FUNCTION();
    auto str = rstr;
    _threadQueue.push_back([str, this, x, y] {
        _tv();
        ::move(y, x);
        for (size_t tx = x, i = 0; i < str.size() && tx < width(); ++tx, ++i) {
            auto c = str.at(i);
            attron(COLOR_PAIR(c.f));
            if (c.c == '\t') {
                ::printw("%s",
                         std::string{std::string(_tabWidth, ' ')}.c_str());
                tx += _tabWidth - 1;
            }
            else {
                ::printw("%s", std::string{std::string_view{c}}.c_str());
            }
            attroff(COLOR_PAIR(c.f));
        }
    });
}

void NCursesScreen::refresh() {
    PROFILE_FUNCTION();
    _threadQueue.push_back([this] {
        _tv();
        ::refresh();
    });
}

void NCursesScreen::clear() {
    PROFILE_FUNCTION();
    _threadQueue.push_back([this] {
        _tv();
        ::clear();
    });
}

void NCursesScreen::cursor(size_t x, size_t y) {
    PROFILE_FUNCTION();
    _threadQueue.push_back([this, x, y] {
        _tv();
        ::move(y, x);
    });
}

NCursesScreen::NCursesScreen()
    : _ncursesThread([this] {
        setThreadName("ncurses thread");
        init();
        _isRunning = true;
        loop();
    }) {
    PROFILE_FUNCTION();

    using namespace std::literals;

    while (!_isRunning) {
        std::this_thread::sleep_for(10ms);
    }
}

NCursesScreen::~NCursesScreen() {
    _threadQueue.push_back([this] {
        ::endwin();
        system("reset");
        _isRunning = false;
    });

    _ncursesThread.join();
}

void NCursesScreen::subscribe(CallbackT f) {
    PROFILE_FUNCTION();
    _callback = f;

    auto r = ResizeEvent();
    r.width = ::COLS; // * 2; // TODO: Find out why * 2
    r.height = ::LINES;
    _callback({r});
}

Event NCursesScreen::getInput() {
    PROFILE_FUNCTION();
    _tv();
    wint_t input = 0;
    const auto result = get_wch(&input);
    if (result == ERR) {
        return NullEvent{};
    }
    const auto c = static_cast<int>(input);

    if (result == KEY_CODE_YES && c == KEY_RESIZE) {
        ::endwin();
        ::refresh();
        ::clear();
        init();
        auto r = ResizeEvent();
        r.width = ::COLS; // * 2; // TODO: Find out why * 2
        r.height = ::LINES;
        return r;
    }

    if (result == KEY_CODE_YES && c == KEY_MOUSE) {
        MEVENT event;
        getmouse(&event);

        if (event.id == -1) {
            return NullEvent{};
        }

        return MouseDownEvent{1, event.x, event.y};
    }

    if (result == KEY_CODE_YES && c == 409) { // Seems to come with scroll wheel
        return NullEvent{};
    }

    else if (auto f = keytranslations.find(c);
             f != keytranslations.end() &&
             (result == KEY_CODE_YES || c < 128)) {
        return KeyEvent{f->second.key, f->second.text};
    }
    else if (c < 27) { // ctrl-characters
        // ctrl+backspace -> 8
        if (c == 8) {
            return KeyEvent{Key::KeyCombination, '\b', Modifiers::Ctrl};
        }

        // Note that ctrl+space -> 0
        return KeyEvent{Key::KeyCombination,
                        c == 0 ? ' ' : static_cast<char>(c + 'A' - 1),
                        Modifiers::Ctrl};
    }
    else if (c >= 28 && c <= 31) {
        // For some reason ctrl+shift+7 is 31
        return KeyEvent{Key::KeyCombination,
                        static_cast<char>('4' + c - 28),
                        Modifiers::Ctrl};
    }
    else if (result == OK) {
        // get_wch returns a Unicode character, not one byte of UTF-8.
        char bytes[MB_LEN_MAX] = {};
        std::mbstate_t state{};
        const auto count = std::wcrtomb(bytes, static_cast<wchar_t>(input), &state);
        if (count == static_cast<size_t>(-1) || count > 4) {
            return NullEvent{};
        }
        return KeyEvent{Key::Text, Utf8Char{std::string_view{bytes, count}}};
    }
    return NullEvent{};
}

size_t NCursesScreen::width() const {
    return static_cast<size_t>(::COLS);
}

size_t NCursesScreen::height() const {
    return static_cast<size_t>(::LINES);
}

void NCursesScreen::title(std::string title) {
    // Not implemented
}

void NCursesScreen::palette(const Palette &palette) {
    PROFILE_FUNCTION();
    _palette = palette;
    _threadQueue.push_back([this] {
        _tv();
        _palette.update(*this);
    });
}

void NCursesScreen::cursorStyle(CursorStyle style) {
    PROFILE_FUNCTION();
    if (_currentCursor != style) {
        // Not implemented
    }
}

size_t NCursesScreen::addStyle(const Color &fg, const Color &bg, size_t index) {
    PROFILE_FUNCTION();
    _tv();
    if (!_hasColors) {
        return 1;
    }

    ++_lastColor;

    ::init_color(_lastColor,
                 fg.r() * 1000 / 255,
                 fg.g() * 1000 / 255,
                 fg.b() * 1000 / 255);

    ++_lastColor;

    ::init_color(_lastColor,
                 bg.r() * 1000 / 255,
                 bg.g() * 1000 / 255,
                 bg.b() * 1000 / 255);

    if (index == std::numeric_limits<size_t>::max()) {
        ++_lastStyle;
        ::init_pair(_lastStyle, _lastColor - 1, _lastColor);
        return _lastStyle;
    }
    else {
        ::init_pair(index, _lastColor - 1, _lastColor);
        return index;
    }
}

void NCursesScreen::loop() {
    PROFILE_FUNCTION();
    using namespace std::literals;
    for (; _isRunning;) {

        auto list = EventListT{};

        for (Event e; e = getInput(), !std::holds_alternative<NullEvent>(e);) {
            list.push_back(std::move(e));
        }

        if (_callback && !list.empty()) {
            _callback(std::move(list));
        }

        for (; !_threadQueue.empty(); _threadQueue.pop_front()) {
            _threadQueue.front()();
        }

        std::this_thread::sleep_for(1ms);
    }
}

#endif
