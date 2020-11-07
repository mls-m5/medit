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

    if (argc > 1) {
        mainWindow.open(argv[1]);
    }
    else {
        mainWindow.updateLocatorBuffer();
    }

    mainWindow.resize();
    mainWindow.draw(*screen);
    mainWindow.updateCursor(*screen);

    while (!shouldQuit) {
        auto c = input->getInput();
        mainWindow._env.key(c);
        mainWindow.keyPress(mainWindow._env);
        mainWindow.resize();

        screen->clear();
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
