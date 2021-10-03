#include "main.h"
#include "core/context.h"
#include "core/jobqueue.h"
#include "core/os.h"
#include "core/registerdefaultplugins.h"
#include "core/timer.h"
#include "files/file.h"
#include "screen/bufferedscreen.h"
#include "screen/guiscreen.h"
#include "screen/htmlscreen.h"
#include "screen/ncursesscreen.h"
#include "views/mainwindow.h"
#include <string>
#include <thread>
#include <vector>

namespace {

void refreshScreen(IWindow &window, IScreen &screen) {
    screen.clear();
    window.draw(screen);

    window.updateCursor(screen);

    screen.refresh();
};

void handleKey(KeyEvent c, MainWindow &mainWindow, IScreen &screen) {
    if (c == Key::Resize) {
        mainWindow.resize(screen.width(), screen.height());
    }
    else {
        mainWindow._env->key(c);
        mainWindow.keyPress(mainWindow._env);
        mainWindow.resize();
    }

    refreshScreen(mainWindow, screen);
};

enum class UiStyle {
    Standard,
    Matgui,
};

struct Settings {
    filesystem::path file;
    UiStyle style = UiStyle::Standard;

    Settings(int argc, char **argv) {
        if (argc < 1) {
            return;
        }
        auto args = std::vector<std::string>{argv + 1, argv + argc};

        for (size_t i = 0; i < args.size(); ++i) {
            auto arg = args.at(i);

            if (arg == "--gui") {
                style = UiStyle::Matgui;
            }
            else {
                if (arg.rfind("-") != 0) {
                    file = args.at(i);
                }
            }
        }
    }
};

std::unique_ptr<IJobQueue> createJobQueue() {
    return std::make_unique<JobQueue>();
}

std::unique_ptr<ITimer> createTimer() {
    return std::make_unique<Timer>();
}

} // namespace

int main(int argc, char **argv) {
    registerDefaultPlugins();
    const auto settings = Settings{argc, argv};

    std::unique_ptr<IScreen> screen;
    auto input = (IInput *){};
#ifdef __EMSCRIPTEN__
    auto ns = std::make_unique<HtmlScreen>();
    input = ns.get();
#else
    auto ns = [&input, style = settings.style]() -> std::unique_ptr<IScreen> {
        if (style == UiStyle::Standard) {
            auto ns = std::make_unique<NCursesScreen>();
            input = ns.get();
            return ns;
        }
        else {
            auto ns = std::make_unique<GuiScreen>();
            input = ns.get();
            return ns;
        }
    }();
#endif
    auto bs = std::make_unique<BufferedScreen>(ns.get(), input);

    screen = std::move(bs);

    auto queue = createJobQueue();
    auto guiQueue = createJobQueue();
    auto timer = createTimer();
    Context context(*queue, *guiQueue, *timer);

    MainWindow mainWindow(*screen, context);

    context.refreshScreenFunc([&] {
        guiQueue->addTask([&] {
            refreshScreen(mainWindow, *screen); //
        });
    });

    if (settings.file.empty()) {
        mainWindow.updateLocatorBuffer();
    }
    else {
        mainWindow.open(settings.file);
    }

    mainWindow.resize();
    mainWindow.draw(*screen);
    mainWindow.updateCursor(*screen);
    screen->refresh();

    timer->start();
    queue->start();

    while (!medit::main::shouldQuit) {
        auto c = input->getInput();

        // Todo: in the future check main window for unsaved changes here
        // too
        while (c != Key::None) {
            if (c == KeyEvent{Key::KeyCombination, 'W', Modifiers::Ctrl} ||
                c == Key::Quit) {
                medit::main::shouldQuit = true;
                break;
            }

            if (c != Key::Unknown) {
                handleKey(c, mainWindow, *screen);
            }
            c = input->getInput();
        }

        guiQueue->work(false);
    }

    if (Os() != Os::Emscripten) {
        // Emscripten will run on single thread but is event driven
        queue->stop();
        timer->stop();
        guiQueue->stop();
    }

    return 0;
}
