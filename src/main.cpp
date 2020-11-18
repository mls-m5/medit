#include "main.h"
#include "core/context.h"
#include "core/jobqueue.h"
#include "core/timer.h"
#include "files/file.h"
#include "screen/bufferedscreen.h"
#include "screen/ncursesscreen.h"
#include "script/rootenvironment.h"
#include "views/mainwindow.h"
#include <string>
#include <thread>
#include <vector>

namespace {

void refreshScreen(IWindow &window, IScreen &screen) {
    screen.clear();
    window.draw(screen);

    //    screen->draw(40,
    //                 screen->height() - 1,
    //                 ((c.modifiers == Modifiers::Ctrl) ? "ctrl+'" : "'") +
    //                     std::string{c.symbol} + "'" +
    //                     c.symbol.byteRepresentation());

    window.updateCursor(screen);

    screen.refresh();
};

void handleKey(KeyEvent c, MainWindow &mainWindow, IScreen &screen) {
    if (c == Key::Resize) {
        mainWindow.resize(screen.width(), screen.height());
    }
    else {
        mainWindow._env.key(c);
        mainWindow.keyPress(mainWindow._env);
        mainWindow.resize();
    }

    refreshScreen(mainWindow, screen);
};

} // namespace

int main(int argc, char **argv) {
    std::unique_ptr<IScreen> screen;
    IInput *input;
    auto ns = std::make_unique<NCursesScreen>();
    auto bs = std::make_unique<BufferedScreen>(ns.get(), ns.get());

    input = ns.get();
    screen = std::move(bs);

    JobQueue queue;
    JobQueue guiQueue;
    Timer timer;
    Context context(queue, timer);

    MainWindow mainWindow(*screen, context);

    context.refreshScreenFunc([&] {
        guiQueue.addTask([&] {
            refreshScreen(mainWindow, *screen); //
        });
    });

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

    std::thread timerThread([&] { timer.loop(); });
    std::thread jobThread([&] { queue.loop(); });
    std::thread guiThread([&] { guiQueue.loop(); });

    std::thread inputThread([&] {
        while (!medit::main::shouldQuit) {
            auto c = input->getInput();

            // Todo: in the future check main window for unsaved changes here
            // too
            if (c == KeyEvent{Key::KeyCombination, 'W', Modifiers::Ctrl}) {
                break;
            }

            guiQueue.addTask([&, c] {
                handleKey(c, mainWindow, *screen); //
            });
        }

        queue.stop();
        timer.stop();
        guiQueue.stop();
    });

    inputThread.join();
    guiThread.join();
    jobThread.join();
    timerThread.join();

    return 0;
}
