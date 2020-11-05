#include "files/file.h"
#include "screen/ncursesscreen.h"
#include "script/rootenvironment.h"
#include "views/bufferview.h"
#include "views/mainwindow.h"
#include <string>
#include <vector>

namespace {
bool shouldQuit = false;

} // namespace

void quitMedit() {
    shouldQuit = true;
}

int main(int argc, char **argv) {
    std::unique_ptr<IScreen> screen;
    IInput *input;

    auto ns = std::make_unique<NCursesScreen>();
    input = ns.get();
    screen = std::move(ns);

    MainWindow mainWindow(screen->width(), screen->height());

    std::unique_ptr<IFile> file;
    if (argc > 1) {
        file = std::make_unique<File>(argv[1]);
    }

    if (file) {
        file->load(mainWindow._editor.buffer());
        mainWindow._editor.file(std::move(file));
    }

    mainWindow.resize();
    mainWindow.draw(*screen);
    mainWindow.updateCursor(*screen);

    while (!shouldQuit) {
        auto c = input->getInput();
        mainWindow._env.key(c);
        screen->clear();
        mainWindow.keyPress(mainWindow._env);
        mainWindow.resize();

        mainWindow.draw(*screen);

        screen->draw(40,
                     screen->height() - 1,
                     ((c.modifiers == Modifiers::Ctrl) ? "ctrl+'" : "'") +
                         std::string{c.symbol} + "'" +
                         c.symbol.byteRepresentation());

        mainWindow.updateCursor(*screen);

        screen->refresh();
    }

    return 0;
}
