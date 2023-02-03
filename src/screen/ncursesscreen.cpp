#ifndef __EMSCRIPTEN__
#include "screen/ncursesscreen.h"
#include "syntax/color.h"
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
    ::init_pair(IPalette::standard, COLOR_WHITE, COLOR_BLACK);
    ::init_pair(IPalette::selection, COLOR_BLUE, COLOR_WHITE);
    ::init_pair(IPalette::statement, COLOR_RED, COLOR_BLACK);
    ::init_pair(IPalette::identifier, COLOR_RED, COLOR_BLACK);
    ::init_pair(IPalette::comment, COLOR_BLUE, COLOR_BLACK);
    ::init_pair(IPalette::lineNumbers, COLOR_BLUE, COLOR_BLACK);
    ::init_pair(IPalette::currentLine, COLOR_WHITE, COLOR_BLACK);
    ::init_pair(IPalette::string, COLOR_YELLOW, COLOR_BLACK);
    ::init_pair(IPalette::type, COLOR_RED, COLOR_BLACK);
    ::init_pair(IPalette::error, COLOR_BLACK, COLOR_RED);
}

} // namespace

void NCursesScreen::init() {
    ::initscr();

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
    ::timeout(10);
}

void NCursesScreen::forceThread() {
    if (_threadId != std::this_thread::get_id()) {
        throw std::runtime_error{
            "trying to call ncurses screen from non gui thread"};
    }
}

void NCursesScreen::draw(size_t x, size_t y, const FString &str) {
    forceThread();
    ::move(y, x);
    for (size_t tx = 0, i = 0; i < str.size() && tx < width() - this->x();
         ++tx, ++i) {
        auto c = str.at(i);
        attron(COLOR_PAIR(c.f));
        if (c.c == '\t') {
            ::printw("%s", std::string{std::string(_tabWidth, ' ')}.c_str());
            tx += _tabWidth - 1;
        }
        else {
            ::printw("%s", std::string{std::string_view{c}}.c_str());
        }
        attroff(COLOR_PAIR(c.f));
    }
}

void NCursesScreen::refresh() {
    forceThread();
    ::refresh();
}

void NCursesScreen::clear() {
    forceThread();
    ::clear();
}

void NCursesScreen::cursor(size_t x, size_t y) {
    forceThread();
    ::move(y, x);
}

NCursesScreen::NCursesScreen() {
    init();
    _threadId = std::this_thread::get_id();
}

NCursesScreen::~NCursesScreen() {
    ::endwin();
    system("reset");
}

Event NCursesScreen::getInput() {
    const auto c = getch();
    if (c == ERR) {
        return NullEvent{};
    }
    else if (c == KEY_RESIZE) {
        ::endwin();
        ::refresh();
        ::clear();
        init();
        return KeyEvent{Key::Resize};
    }
    else if (auto f = keytranslations.find(c); f != keytranslations.end()) {
        return KeyEvent{f->second.key, f->second.text};
    }
    else if (c < 27) { // ctrl-characters
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
    else if ((c & 0b11111111) == c) {
        Utf8Char uc{static_cast<char>(c)};
        auto size = utf8size(static_cast<char>(c));
        for (size_t i = 1; i < size; ++i) {
            uc[i] = static_cast<char>(getch());
        }

        return KeyEvent{Key::Text, uc};
    }
    else {
        return KeyEvent{Key::Text, static_cast<char>(c)};
    }
}

size_t NCursesScreen::x() const {
    return 0;
}

size_t NCursesScreen::y() const {
    return 0;
}

size_t NCursesScreen::width() const {
    return static_cast<size_t>(::COLS *
                               2); // TODO: Figure out why 2 is required here
}

size_t NCursesScreen::height() const {
    return static_cast<size_t>(::LINES);
}

void NCursesScreen::title(std::string title) {
    // Not implemented
}

void NCursesScreen::cursorStyle(CursorStyle style) {
    forceThread();
    if (_currentCursor != style) {
        // Not implemented
    }
}

size_t NCursesScreen::addStyle(const Color &fg, const Color &bg, size_t index) {
    forceThread();
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

#endif
