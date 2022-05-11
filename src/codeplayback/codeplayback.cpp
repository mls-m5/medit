
#include "screen/guiscreen.h"
#include "text/cursorops.h"
#include "views/editor.h"
#include <iostream>
#include <thread>

using namespace std::literals;

auto testText = R"_(

int main() {
    std::cout << "hello\n";
}

)_"sv;

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
            screen.cursorStyle(CursorStyle::Block);

            for (auto c : testText) {
                screen.cursorStyle(CursorStyle::Beam);
                insert(c, buffer.end());
                editor.draw(screen);
                //                editor.bufferView().cursorPosition(buffer.end());
                editor.cursor(buffer.end());
                editor.updateCursor(screen);
                screen.refresh();
                std::this_thread::sleep_for(30ms);
            }
            screen.cursorStyle(CursorStyle::Block);
            screen.refresh();
        }
    }

    return 0;
}
