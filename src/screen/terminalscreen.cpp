
#include "screen/terminalscreen.h"
#include <ncurses.h>

void TerminalScreen::draw(size_t x, size_t y, FString str) {
    // Todo: Fix formatting
    ::mvprintw(x, y, std::string{str}.c_str());
}

void TerminalScreen::refresh() {
    ::refresh();
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
