
#include "screen/terminalscreen.h"
#include <ncurses.h>

void TerminalScreen::draw(size_t x, size_t y, FString str) {
    // Todo: Fix formatting
    ::mvprintw(y, x, std::string{str}.c_str());
}

void TerminalScreen::refresh() {
    ::refresh();
}

void TerminalScreen::clear() {
    ::clear();
}

void TerminalScreen::cursor(size_t x, size_t y) {
    ::move(y, x);
}

TerminalScreen::TerminalScreen() {
    ::initscr();
    ::raw();
    ::keypad(stdscr, true);
    ::noecho();
}

KeyEvent TerminalScreen::getInput() {
    auto c = getch();
    return KeyEvent{.key = c};
}

size_t TerminalScreen::x() const {
    return 0;
}

size_t TerminalScreen::y() const {
    return 0;
}

size_t TerminalScreen::width() const {
    return ::COLS;
}

size_t TerminalScreen::height() const {
    return ::LINES;
}
