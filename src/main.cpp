
#include "files/file.h"
#include "modes/insertmode.h"
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
    NCursesScreen screen;

    Editor editor;
    Environment env;
    env.editor(&editor);

    editor.mode(std::make_unique<InsertMode>(env));

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
        env.key(c);
        screen.clear();
        editor.keyPress(c);

        editor.draw(screen);

        screen.draw(40,
                    screen.height() - 1,
                    std::string{c.symbol} + c.symbol.byteRepresentation());

        editor.updateCursor(screen);

        screen.refresh();
    }

    return 0;
}
