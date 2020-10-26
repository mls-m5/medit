
#include "editor.h"
#include "screen/ncursesscreen.h"
#include "views/bufferview.h"
#include <string>
#include <vector>

struct Screen {
    int width = 0;
    int height = 0;
} screen;

int main(int /*argc*/, char ** /*argv*/) {
    NCursesScreen screen;

    Editor editor;

    while (true) {
        auto c = screen.getInput();
        screen.clear();
        editor.keyPress(c);

        screen.draw(10, 10, std::string{"hej"});
        editor.draw(screen);

        screen.draw(
            40, screen.height() - 1, std::to_string(static_cast<int>(c.key)));

        editor.updateCursor(screen);

        screen.refresh();
    }

    return 0;
}
