#include "main.h"
#include "core/context.h"
#include "core/jobqueue.h"
#include "core/jsjobqueue.h"
#include "core/jstimer.h"
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

#ifdef __EMSCRIPTEN__

#include "emscripten.h"

#endif

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

void innerMainLoop(IInput &input,
                   IJobQueue &guiQueue,
                   std::function<void(KeyEvent)> callback) {

    auto c = input.getInput();

    // Todo: in the future check main window for unsaved changes
    // here too
    while (c != Key::None) {
        if (c == KeyEvent{Key::KeyCombination, 'W', Modifiers::Ctrl} ||
            c == Key::Quit) {
            medit::main::shouldQuit = true;
            break;
        }

        if (c != Key::Unknown) {
            callback(c);
        }
        c = input.getInput();
    }

    guiQueue.work(false);
};

#ifdef __EMSCRIPTEN__

struct EmscriptenData {
    std::shared_ptr<IScreen> nativeScreen;
    std::shared_ptr<IScreen> screen;
    std::shared_ptr<IJobQueue> guiQueue;
    std::shared_ptr<IJobQueue> queue;
    std::shared_ptr<ITimer> timer;
    std::shared_ptr<ITimer> guiLoopTimer;
};

std::function<void()> emCallback;

EmscriptenData emData;

int mainFunc(int argc, char **argv) {
    registerDefaultPlugins();
    const auto settings = Settings{0, nullptr};
    auto input = (IInput *){};

    auto ns = std::make_unique<HtmlScreen>();
    //    auto ns = std::make_unique<GuiScreen>();
    input = ns.get();

    auto bs = std::make_unique<BufferedScreen>(ns.get(), input);

    std::shared_ptr<IScreen> screen = std::move(bs);

    auto queue = std::make_shared<JsJobQueue>();
    auto guiQueue = std::make_shared<JsJobQueue>();
    auto timer = std::make_shared<JsTimer>();
    auto context = std::make_shared<Context>(*queue, *guiQueue, *timer);

    auto mainWindow = std::make_shared<MainWindow>(*screen, *context);

    context->refreshScreenFunc([=] {
        guiQueue->addTask([=] {
            refreshScreen(*mainWindow, *screen); //
        });
    });

    if (settings.file.empty()) {
        mainWindow->updateLocatorBuffer();
    }
    else {
        mainWindow->open(settings.file);
    }

    mainWindow->resize();
    mainWindow->draw(*screen);
    mainWindow->updateCursor(*screen);
    screen->refresh();

    timer->start();
    queue->start();

    auto callback = [=](KeyEvent c) { handleKey(c, *mainWindow, *screen); };

    using namespace std::chrono_literals;

    auto guiLoopTimer = std::make_shared<JsTimer>();

    emCallback = [=] {
        innerMainLoop(*input, *guiQueue, callback);
        guiLoopTimer->setTimeout(
            10ms, emCallback); // Todo: Do this faster when it works
                               //        EM_ASM(console.log("inner loop"));
    };

    guiLoopTimer->start();
    guiLoopTimer->setTimeout(100ms, emCallback);

    EM_ASM(console.log("hej"));

    emData.nativeScreen = std::move(ns);
    emData.screen = screen;
    emData.guiQueue = guiQueue;
    emData.queue = queue;
    emData.timer = timer;
    emData.guiLoopTimer = guiLoopTimer;
    return 0;
}

#else  // __EMSCRIPTEN

int mainFunc(int argc, char **argv) {
    registerDefaultPlugins();
    const auto settings = Settings{argc, argv};

    auto input = (IInput *){};
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

    auto bs = std::make_unique<BufferedScreen>(ns.get(), input);

    std::unique_ptr<IScreen> screen = std::move(bs);

    auto queue = std::make_unique<JobQueue>();
    auto guiQueue = std::make_unique<JobQueue>();
    auto timer = std::make_unique<Timer>();
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

    auto callback = [&](KeyEvent c) { handleKey(c, mainWindow, *screen); };

    // If multithreaded lock gui thread with this
    while (!medit::main::shouldQuit) {
        innerMainLoop(*input, *guiQueue, callback);
    }

    queue->stop();
    timer->stop();
    guiQueue->stop();

    return 0;
}
#endif // __EMSCRIPTEN__

} // namespace

int main(int argc, char **argv) {
    mainFunc(argc, argv);

    return 0;
}
