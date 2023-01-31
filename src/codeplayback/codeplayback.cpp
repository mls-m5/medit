
#include "text/cursorops.h"
#include "views/editor.h"
#include <iostream>
#include <thread>

#ifdef __EMSCRIPTEN__

#include "screen/htmlscreen.h"
using ScreenType = HtmlScreen;

#else

#include "screen/guiscreen.h"
using ScreenType = GuiScreen;

#endif

using namespace std::literals;

auto testText = R"_(

int main() {
    std::cout << "hello\n";
}

)_"sv;

int main(int argc, char *argv[]) {
    auto screen = ScreenType{};

    std::cout << "hello\n";

    auto c = screen.getInput();

    auto editor = Editor{};

    auto &buffer = editor.buffer();
    buffer.insert(0, "hello");

    editor.width(screen.width());
    editor.height(screen.height() - 1);

    //    auto lastChar = 0;
    auto wasAlpha = false;

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
                auto a = isalpha(c);
                //                if (!isalpha(c) && c != lastChar) {
                if (!a && a != wasAlpha) {
                    std::this_thread::sleep_for(100ms);
                }
                else {
                    std::this_thread::sleep_for(20ms);
                }
                //                lastChar = c;
                wasAlpha = a;
            }
            screen.cursorStyle(CursorStyle::Block);
            screen.refresh();
        }
    }

    return 0;
}
