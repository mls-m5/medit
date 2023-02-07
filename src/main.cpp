#include "main.h"
#include "core/context.h"
#include "core/coreenvironment.h"
#include "core/debugoutput.h"
#include "core/jobqueue.h"
#include "core/jsjobqueue.h"
#include "core/jstimer.h"
#include "core/os.h"
#include "core/timer.h"
#include "files/file.h"
#include "registerdefaultplugins.h"
#include "screen/bufferedscreen.h"
#include "settings.h"
#include "thinmain.h"
#include "views/mainwindow.h"
#include <string>
#include <thread>
#include <vector>

#ifdef __EMSCRIPTEN__

#include "screen/htmlscreen.h"
// #include <emscripten.h>

using ScreenType = HtmlScreen;
using QueueType = JsJobQueue;
using TimerType = JsTimer;

#else

#include "remote/fifofile.h"
#include "remote/fiforeceiver.h"
#include "screen/deserializescreen.h"
#include "screen/guiscreen.h"
#include "screen/ncursesscreen.h"
#include "screen/serializescreen.h"
using ScreenType = GuiScreen;
using QueueType = JobQueue;
using TimerType = Timer;

#endif

namespace {

struct MainData {
    std::shared_ptr<IScreen> nativeScreen;
    std::shared_ptr<IScreen> screen;
    std::shared_ptr<IJobQueue> guiQueue;
    std::shared_ptr<IJobQueue> queue;
    std::shared_ptr<ITimer> timer;
    std::shared_ptr<ITimer> guiLoopTimer;
    std::shared_ptr<Context> context;
    std::shared_ptr<MainWindow> mainWindow;

    MainData() = default; // Static initialization

    void start(const Settings &settings);

    void loop();

    void stop();

    void callback(Event c) {
        handleKey(c, *mainWindow, *screen);
    }

    void createScreen(const Settings &settings) {
#ifdef __EMSCRIPTEN__
        nativesScreen = std::make_unique<ScreenType>();
#else
        if (settings.style == UiStyle::Terminal) {
            nativeScreen = std::make_unique<NCursesScreen>();
        }
        else if (settings.style == UiStyle::Remote) {
            auto deserializeScreen = std::make_shared<DeserializeScreen>(
                std::make_unique<ScreenType>());

            nativeScreen = std::make_unique<SerializeScreen>(deserializeScreen);
        }
        else if (settings.style == UiStyle::FifoServer) {
            std::cout << "starting fifo server..." << std::endl;
            auto fifo = std::make_shared<FifoConnection>(
                FifoFile::standardClientOut(), FifoFile::standardClientIn());
            nativeScreen = std::make_unique<SerializeScreen>(fifo);
        }
        else {
            nativeScreen = std::make_unique<ScreenType>();
        }
#endif
        nativeScreen->subscribe([this](IScreen::EventListT list) {
            this->guiQueue->addTask([e = list, this] {
                handleEvent(e);
                if (medit::main::shouldQuit) {
                    return;
                }
            });
        });
    }

    void handleKey(Event e, MainWindow &mainWindow, IScreen &screen) {
        // TODO: Consider moving this function to mainWindow
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
        if (auto ce = std::get_if<MouseDownEvent>(&e)) {
            mainWindow.mouseDown(ce->x, ce->y);
        }
    }

    void handleEvent(const IScreen::EventListT &list) {
        for (auto &c : list) {
            if (std::holds_alternative<NullEvent>(c)) {
                return;
            }

            if (auto key = std::get_if<KeyEvent>(&c)) {
                if (*key ==
                        KeyEvent{Key::KeyCombination, 'W', Modifiers::Ctrl} ||
                    *key == Key::Quit) {
                    medit::main::shouldQuit = true;
                    guiQueue->stop();
                }

                if (*key != Key::Unknown) {
                    callback(*key);
                }
            }
            else {
                callback(c);
            }
            mainWindow->triggerRedraw();
        }
    }
};

MainData mainData;

void MainData::start(const Settings &settings) {
    registerDefaultPlugins();

    queue = std::make_shared<QueueType>();
    guiQueue = std::make_shared<QueueType>();
    timer = std::make_shared<TimerType>();

    createScreen(settings);

    screen = std::make_unique<BufferedScreen>(nativeScreen.get());

    context = std::make_shared<Context>(*queue, *guiQueue, *timer);

    // TODO: Core context should probably live in this file somewhere
    CoreEnvironment::instance().context(context.get());

    mainWindow = std::make_shared<MainWindow>(*screen, *context);

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
    queue->stop();
    timer->stop();
    guiQueue->stop();
}

void MainData::loop() {

#ifdef __EMSCRIPTEN__
    guiQueue->start();
#else
    while (!medit::main::shouldQuit) {
        guiQueue->work(true);
        mainWindow->refreshScreen();
    }
#endif
}

} // namespace

int main(int argc, char **argv) {
    auto settings = Settings{argc, argv};

#ifndef __EMSCRIPTEN__
    if (settings.style == UiStyle::FifoClient ||
        settings.style == UiStyle::TcpServer) {
        return thinMain(settings);
    }
#endif

    mainData.start(settings);
    mainData.loop();
#ifndef __EMSCRIPTEN__
    mainData.stop();
#endif

    return 0;
}
