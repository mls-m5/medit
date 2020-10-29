
#include "files/file.h"
#include "modes/normalmode.h"
#include "screen/linuxterminalscreen.h"
#include "screen/ncursesscreen.h"
#include "script/environment.h"
#include "views/bufferview.h"
#include "views/editor.h"
#include <string>
#include <vector>

struct Screen {
    int width = 0;
    int height = 0;
} screen;

int main(int argc, char **argv) {
    std::unique_ptr<IScreen> screen;
    IInput *input;

    if (true) {
        auto ns = std::make_unique<NCursesScreen>();
        input = ns.get();
        screen = std::move(ns);
    }
    else {
        auto ls = std::make_unique<LinuxTerminalScreen>();
        input = ls.get();
        screen = std::move(ls);
    }

    Editor editor;
    Environment env;
    env.editor(&editor);

    editor.mode(createNormalMode());

    std::unique_ptr<IFile> file;
    if (argc > 1) {
        file = std::make_unique<File>(argv[1]);
    }

    if (file) {
        file->load(editor.buffer());
    }

    editor.draw(*screen);
    editor.updateCursor(*screen);

    while (true) {
        auto c = input->getInput();
        env.key(c);
        screen->clear();
        editor.keyPress(env);

        editor.draw(*screen);

        screen->draw(40,
                     screen->height() - 1,
                     std::string{c.symbol} + c.symbol.byteRepresentation());

        editor.updateCursor(*screen);

        screen->refresh();
    }

    return 0;
}
