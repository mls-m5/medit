#include "main.h"
#include "core/context.h"
#include "core/coreenvironment.h"
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

void handleKey(Event e, MainWindow &mainWindow, IScreen &screen) {
    if (auto c = std::get_if<KeyEvent>(&e)) {
        if (*c == Key::Resize) {
            mainWindow.resize(screen.width(), screen.height());
        }
        else {
            mainWindow._env->key(*c);
            mainWindow.keyPress(mainWindow._scope);
            mainWindow.resize();
        }
    }
    if (auto p = std::get_if<PasteEvent>(&e)) {
        mainWindow.paste(p->text);
    }
    if (auto ce = std::get_if<CopyEvent>(&e)) {
        auto text = mainWindow.copy(ce->shouldCut);
        ce->callback(text);
    }

    // TODO: Update when open buffers change instead
    refreshScreen(mainWindow, screen);
}

void innerMainLoop(IInput &input,
                   IJobQueue &guiQueue,
                   std::function<void(Event)> callback) {

    auto c = input.getInput();

    // Todo: in the future check main window for unsaved changes
    // here too
    while (!std::holds_alternative<NullEvent>(c)) {
        if (auto key = std::get_if<KeyEvent>(&c)) {
            if (*key == KeyEvent{Key::KeyCombination, 'W', Modifiers::Ctrl} ||
                *key == Key::Quit) {
                medit::main::shouldQuit = true;
                break;
            }

            if (*key != Key::Unknown) {
                callback(*key);
            }
        }
        else {
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
    std::shared_ptr<Context> context;
    std::shared_ptr<MainWindow> mainWindow;

    IInput *input = {};

    MainData() = default; // Static initialization

    void start(int argc, char **argv);

    void loop();

    void stop();

    void callback(Event c) {
        handleKey(c, *mainWindow, *screen);
    }

    void createScreen(const Settings &settings) {
        //        if (settings.style == UiStyle::Terminal) {
        //            auto ns = std::make_unique<NCursesScreen>();
        //            input = ns.get();
        //            nativeScreen = std::move(ns);
        //        }
        //        else {
        auto ns = std::make_unique<ScreenType>();
        input = ns.get();
        nativeScreen = std::move(ns);
        //        }
    }
};

MainData mainData;

void MainData::start(int argc, char **argv) {
    registerDefaultPlugins();
    const auto settings = Settings{argc, argv};

    createScreen(settings);

    screen = std::make_unique<BufferedScreen>(nativeScreen.get(), input);

    queue = std::make_shared<QueueType>();
    guiQueue = std::make_shared<QueueType>();
    timer = std::make_shared<TimerType>();

    context = std::make_shared<Context>(*queue, *guiQueue, *timer);

    // TODO: Core context should probably live in this file somewhere
    CoreEnvironment::instance().context(context.get());

    mainWindow = std::make_shared<MainWindow>(*screen, *context);

    context->refreshScreenFunc([] {
        mainData.guiQueue->addTask([] {
            refreshScreen(*mainData.mainWindow, *mainData.screen); //
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
}

void MainData::stop() {
#ifndef __EMSCRIPTEN__
    queue->stop();
    timer->stop();
    guiQueue->stop();
#endif
}

void MainData::loop() {
#ifdef __EMSCRIPTEN__
    using namespace std::chrono_literals;
    guiLoopTimer = std::make_shared<JsTimer>();

    emCallback = [=] {
        innerMainLoop(*input, *guiQueue, [](auto c) { mainData.callback(c); });
        guiLoopTimer->setTimeout(
            10ms, emCallback); // Todo: Do this faster when it works
    };

    guiLoopTimer->start();
    guiLoopTimer->setTimeout(100ms, emCallback);
#else
    while (!medit::main::shouldQuit) {
        innerMainLoop(*input, *guiQueue, [](auto c) { mainData.callback(c); });
    }
#endif
}

} // namespace

int main(int argc, char **argv) {
    mainData.start(argc, argv);
    mainData.loop();
    mainData.stop();

    return 0;
}
