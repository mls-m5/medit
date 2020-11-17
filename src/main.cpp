#include "main.h"
#include "core/jobqueue.h"
#include "files/file.h"
#include "screen/bufferedscreen.h"
#include "screen/ncursesscreen.h"
#include "script/rootenvironment.h"
#include "views/mainwindow.h"
#include <string>
#include <thread>
#include <vector>

int main(int argc, char **argv) {
    std::unique_ptr<IScreen> screen;
    IInput *input;
    auto ns = std::make_unique<NCursesScreen>();
    auto bs = std::make_unique<BufferedScreen>(ns.get(), ns.get());

    input = ns.get();
    screen = std::move(bs);

    MainWindow mainWindow(*screen);

    if (argc > 1) {
        mainWindow.open(argv[1]);
    }
    else {
        mainWindow.updateLocatorBuffer();
    }

    mainWindow.resize();
    mainWindow.draw(*screen);
    mainWindow.updateCursor(*screen);
    screen->refresh();

    JobQueue queue;
    std::thread inputThread([&] {
        while (!medit::main::shouldQuit) {
            auto c = input->getInput();

            if (c == Key::Resize) {
                mainWindow.resize(screen->width(), screen->height());
            }
            else {
                mainWindow._env.key(c);
                mainWindow.keyPress(mainWindow._env);
                mainWindow.resize();
            }

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

        queue.stop();
    });

    queue.loop();
    inputThread.join();

    return 0;
}
