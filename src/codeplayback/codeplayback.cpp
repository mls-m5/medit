
#include "screen/guiscreen.h"
#include "text/cursorops.h"
#include "views/editor.h"
#include <iostream>

int main(int argc, char *argv[]) {
    auto screen = GuiScreen{};

    std::cout << "hello\n";

    auto c = screen.getInput();

    auto editor = Editor{};

    auto &buffer = editor.buffer();
    buffer.insert(0, "hello");

    editor.width(screen.width());
    editor.height(screen.height());

    for (; c != Key::Quit; c = screen.getInput()) {
        if (c != Key::None && c != Key::Unknown) {
            editor.draw(screen);

            for (auto c : "hello") {
                insert(c, buffer.end());
            }

            screen.refresh();
        }
    }

    return 0;
}
