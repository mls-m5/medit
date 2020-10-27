
#include "editor.h"
#include "files/file.h"
#include "modes/insertmode.h"
#include "screen/ncursesscreen.h"
#include "views/bufferview.h"
#include <string>
#include <vector>

struct Screen {
    int width = 0;
    int height = 0;
} screen;

int main(int argc, char **argv) {
    NCursesScreen screen;

    Editor editor;
    InsertMode insertMode;
    editor.mode(&insertMode);

    std::unique_ptr<IFile> file;
    if (argc > 1) {
        file = std::make_unique<File>(argv[1]);
    }

    if (file) {
        file->load(editor.buffer());
    }

    editor.draw(screen);
    while (true) {
        auto c = screen.getInput();
        screen.clear();
        editor.keyPress(c);

        editor.draw(screen);

        screen.draw(40, screen.height() - 1, std::string{c.symbol});

        editor.updateCursor(screen);

        screen.refresh();
    }

    return 0;
}
