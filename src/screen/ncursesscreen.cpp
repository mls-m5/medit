
#include "screen/ncursesscreen.h"
#include <map>
#include <ncurses.h>

namespace {

struct KeyTranslation {
    Key key;
    Utf8Char text{};
};

//! Keys that starts with ^
const auto keytranslations = std::map<int16_t, KeyTranslation>{
    {263, {Key::Backspace}},
    {10, {Key::Return, "\n"}},
    {258, {Key::Down}},
    {259, {Key::Up}},
    {260, {Key::Left}},
    {261, {Key::Right}},
    {27, {Key::Escape}},
};

} // namespace

void NCursesScreen::draw(size_t x, size_t y, const FString &str) {
    // Todo: Fix formatting
    ::mvprintw(y, x, std::string{str}.c_str());
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
    ::initscr();
    ::raw();
    ::keypad(stdscr, true);
    ::noecho();

    ::start_color();
    ::init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    ::init_pair(2, COLOR_RED, COLOR_MAGENTA);
    ::init_pair(3, COLOR_CYAN, COLOR_BLACK);
    ::init_pair(4, COLOR_BLACK, COLOR_WHITE);
}

KeyEvent NCursesScreen::getInput() {
    const auto c = getch();
    if (auto f = keytranslations.find(c); f != keytranslations.end()) {
        return KeyEvent{f->second.key, f->second.text};
    }
    else if ((c & 0b11111111) == c) {
        Utf8Char uc{static_cast<char>(c)};
        auto size = utf8size(c);
        for (size_t i = 1; i < size; ++i) {
            uc[i] = getch();
        }

        return KeyEvent{Key::Text, uc};
    }
    else {
        return KeyEvent{Key::Text, c};
    }
}

size_t NCursesScreen::x() const {
    return 0;
}

size_t NCursesScreen::y() const {
    return 0;
}

size_t NCursesScreen::width() const {
    return ::COLS;
}

size_t NCursesScreen::height() const {
    return ::LINES;
}
