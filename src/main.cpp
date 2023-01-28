#include "main.h"
#include "core/context.h"
#include "core/jobqueue.h"
#include "core/jsjobqueue.h"
#include "core/jstimer.h"
#include "core/os.h"
#include "core/timer.h"
#include "files/file.h"
#include "registerdefaultplugins.h"
#include "screen/bufferedscreen.h"
#include "screen/guiscreen.h"
#include "screen/htmlscreen.h"
#include "screen/ncursesscreen.h"
#include "settings.h"
#include "views/mainwindow.h"
#include <string>
#include <thread>
#include <vector>

#ifdef __EMSCRIPTEN__

#include "emscripten.h"

using ScreenType = HtmlScreen;
using QueueType = JsJobQueue;
using TimerType = JsTimer;

#else

using ScreenType = GuiScreen;
using QueueType = JobQueue;
using TimerType = Timer;

#endif

namespace {

void refreshScreen(IWindow &window, IScreen &screen) {
    screen.clear();
    window.draw(screen);

    window.updateCursor(screen);

    screen.refresh();
}

void handleKey(KeyEvent c, MainWindow &mainWindow, IScreen &screen) {
    if (c == Key::Resize) {
        mainWindow.resize(screen.width(), screen.height());
    }
    else {
        mainWindow._env->key(c);
        mainWindow.keyPress(mainWindow._scope);
        mainWindow.resize();
    }

    refreshScreen(mainWindow, screen);
}

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
}

// #ifdef __EMSCRIPTEN__

std::function<void()> emCallback;

struct MainData {
    std::shared_ptr<IScreen> nativeScreen;
    std::shared_ptr<IScreen> screen;
    std::shared_ptr<IJobQueue> guiQueue;
    std::shared_ptr<IJobQueue> queue;
    std::shared_ptr<ITimer> timer;
    std::shared_ptr<ITimer> guiLoopTimer;

    MainData() = default; // Static initialization

    MainData(int argc, char **argv);

    ~MainData();
};

MainData mainData;

MainData::MainData(int argc, char **argv) {
    registerDefaultPlugins();
    const auto settings = Settings{0, nullptr};
    auto input = (IInput *){};

    {
        auto ns = std::make_unique<ScreenType>();
        input = ns.get();
        nativeScreen = std::move(ns);
    }

    screen = std::make_unique<BufferedScreen>(nativeScreen.get(), input);

    queue = std::make_shared<QueueType>();
    guiQueue = std::make_shared<QueueType>();
    timer = std::make_shared<TimerType>();

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

#ifdef __EMSCRIPTEN__
    guiLoopTimer = std::make_shared<JsTimer>();

    emCallback = [=] {
        innerMainLoop(*input, *guiQueue, callback);
        guiLoopTimer->setTimeout(
            10ms, emCallback); // Todo: Do this faster when it works
    };

    guiLoopTimer->start();
    guiLoopTimer->setTimeout(100ms, emCallback);
#else
    while (!medit::main::shouldQuit) {
        innerMainLoop(*input, *guiQueue, callback);
    }
#endif
}

MainData::~MainData() {
    queue->stop();
    timer->stop();
    guiQueue->stop();
}

} // namespace

int main(int argc, char **argv) {
    mainData = MainData(argc, argv);

    return 0;
}
