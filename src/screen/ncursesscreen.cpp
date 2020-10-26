
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
};

} // namespace

void NCursesScreen::draw(size_t x, size_t y, FString str) {
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
}

KeyEvent NCursesScreen::getInput() {
    const auto c = getch();
    if ((c & 0b11111111) == c) {
        Utf8Char uc{static_cast<char>(c)};
        auto size = utf8size(c);
        for (size_t i = 1; i < size; ++i) {
            uc[i] = getch();
        }

        return KeyEvent{.key = Key::Text, .symbol = uc};
    }
    else if (auto f = keytranslations.find(c); f != keytranslations.end()) {
        return KeyEvent{.key = f->second.key, .symbol = f->second.text};
    }
    else {
        return KeyEvent{.key = Key::Text, .symbol = c};
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
