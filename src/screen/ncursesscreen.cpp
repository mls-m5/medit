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
    {KEY_ENTER, {Key::Return, "\n"}},
    {32, {Key::Space, " "}},
    {KEY_DOWN, {Key::Down}},
    {KEY_UP, {Key::Up}},
    {KEY_LEFT, {Key::Left}},
    {KEY_RIGHT, {Key::Right}},
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

void init() {
    ::initscr();
    ::start_color();
    ::raw();
    ::keypad(stdscr, true);
    ::noecho();

    //    init_color(77, 372, 843, 372);
    // init_color seems to take a value from 0 to 1000
    // The first 16 indices are also reserved
    // it could also be good pracice to use ::has_colors()
    //    ::init_color(77, 1000, 0, 0);    // Intensive red
    //    ::init_color(78, 0, 0, 300);     // Dark blue
    //    ::init_color(79, 50, 50, 100);   // Dark blue
    //    ::init_color(80, 400, 500, 500); // Gray
    //    ::init_pair(1, 77, COLOR_BLACK);
    //    //    ::init_pair(1, COLOR_RED, COLOR_BLACK);
    //    ::init_pair(2, COLOR_GREEN, COLOR_BLACK);
    //    ::init_pair(3, COLOR_CYAN, 78); // Line numbers
    //    ::init_pair(4, COLOR_GREEN, COLOR_WHITE);
    //    ::init_pair(5, COLOR_WHITE, 79);
    //    ::init_pair(6, COLOR_WHITE, 80); // split color
}

} // namespace

void NCursesScreen::draw(size_t x, size_t y, const FString &str) {
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
    ::refresh();
}

void NCursesScreen::clear() {
    ::clear();
}

void NCursesScreen::cursor(size_t x, size_t y) {
    ::move(y, x);
}

NCursesScreen::NCursesScreen() {
    init();
}

NCursesScreen::~NCursesScreen() {
    ::endwin();
}

KeyEvent NCursesScreen::getInput() {
    const auto c = getch();
    if (c == KEY_RESIZE) {
        ::endwin();
        ::refresh();
        ::clear();
        init();
        return KeyEvent{Key::Resize};
    }
    if (auto f = keytranslations.find(c); f != keytranslations.end()) {
        return KeyEvent{f->second.key, f->second.text};
    }
    else if (c < 27) { // ctrl-characters
        // Note that ctrl+space -> 0
        return KeyEvent{Key::KeyCombination,
                        c == 0 ? ' ' : static_cast<char>(c + 'A' - 1),
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
    return static_cast<size_t>(::COLS);
}

size_t NCursesScreen::height() const {
    return static_cast<size_t>(::LINES);
}

size_t NCursesScreen::addStyle(const Color &fg, const Color &bg, size_t index) {
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
