
#include "editor.h"
#include <ncurses.h>
#include <string>
#include <vector>

struct Screen {
    int width = 0;
    int height = 0;
} screen;

void printRows() {
    for (int i = 0; i < screen.height; ++i) {
        mvprintw(i, 0, std::to_string(i).c_str());
    }
}

int main(int /*argc*/, char ** /*argv*/) {
    initscr();
    raw();
    keypad(stdscr, true);
    noecho();

    printw("hello");

    refresh();

    //    Buffer buffer;
    Editor editor;

    while (true) {
        auto c = getch();
        attron(A_BOLD);
        printw("%c", c);

        attroff(A_BOLD);
        printw(",");

        //        buffer.insert(c);
        editor.keyPress(c);

        mvprintw(10, 10, "hej");
        mvprintw(9, 10, "%d", c);

        mvprintw(11, 3, editor.buffer().lines().front().c_str());

        refresh();
    }

    endwin();

    return 0;
}
