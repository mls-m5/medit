
#include "editor.h"
//#include <ncurses.h>
#include "screen/terminalscreen.h"
#include <string>
#include <vector>

struct Screen {
    int width = 0;
    int height = 0;
} screen;

// void printRows() {
//    for (int i = 0; i < screen.height; ++i) {
//        mvprintw(i, 0, std::to_string(i).c_str());
//    }
//}

int main(int /*argc*/, char ** /*argv*/) {
    TerminalScreen screen;

    Editor editor;

    while (true) {
        auto c = screen.getInput();
        editor.keyPress(c);

        screen.draw(10, 10, std::string{"hej"});
        screen.draw(11, 3, editor.buffer().lines().front());

        screen.refresh();
    }

    return 0;
}
