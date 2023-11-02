#include "main.h"
#include "core/context.h"
#include "core/coreenvironment.h"
#include "core/jobqueue.h"
#include "core/os.h"
#include "core/profiler.h"
#include "core/threadname.h"
#include "core/timer.h"
#include "files/config.h"
#include "registerdefaultplugins.h"
#include "screen/bufferedscreen.h"
#include "settings.h"
#include "thinmain.h"
#include "views/mainwindow.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#ifdef __EMSCRIPTEN__

#include "core/jsjobqueue.h"
#include "core/jstimer.h"
#include "screen/htmlscreen.h"

using ScreenType = HtmlScreen;
using QueueType = JsJobQueue;
using TimerType = JsTimer;

#else

#include "core/fifofile.h"
#include "remote/fifoconnection.h"
#include "remote/tcpserver.h"
#include "screen/deserializescreen.h"
#include "screen/guiscreen.h"
#include "screen/ncursesscreen.h"
#include "screen/serializescreen.h"

using ScreenType = GuiScreen;
using QueueType = JobQueue;
using TimerType = Timer;

#endif

namespace {

struct User {
    std::shared_ptr<IScreen> nativeScreen;
    std::shared_ptr<IScreen> screen;
    std::shared_ptr<MainWindow> mainWindow;

#ifdef __EMSCRIPTEN__
    User(const Settings &settings, CoreEnvironment &core, Context &context) {
        nativesScreen = std::make_unique<ScreenType>();
    }
#else

    User(const Settings &settings,
         CoreEnvironment &core,
         ThreadContext &context) {
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
                createFifo(fifoClientOutPath()),
                createFifo(fifoClientInPath()));
            nativeScreen = std::make_unique<SerializeScreen>(fifo);
        }
        else if (settings.style == UiStyle::TcpServer) {
            /// Since a tcp server can host multiple clients it needs to be
            /// handled differently
            throw std::runtime_error{"server should be handled differently"};
        }
        else {
            nativeScreen = std::make_unique<ScreenType>();
        }
        setup(settings, core, context);
    }

    User(std::shared_ptr<IConnection> conn,
         CoreEnvironment &core,
         const Settings &settings,
         ThreadContext &context) {
        nativeScreen = std::make_unique<SerializeScreen>(conn);
        setup(settings, core, context);
    }

    void setup(const Settings &settings,
               CoreEnvironment &core,
               ThreadContext &context) {
        screen = std::make_unique<BufferedScreen>(nativeScreen.get());
        screen->subscribe([this, &context](IScreen::EventListT list) {
            context.guiQueue().addTask([e = list, this] {
                handleEvents(e);
                if (medit::main::shouldQuit) {
                    return;
                }
            });
        });

        mainWindow = std::make_shared<MainWindow>(core, *screen, context);

        if (settings.file.empty()) {
            mainWindow->updateLocatorBuffer();
        }
        else {
            mainWindow->open(settings.file);
            mainWindow->currentEditor()->removeEmptyUnsavedBufferHistory();
        }

        screen->refresh();
    }

#endif

    void handleEvents(const IScreen::EventListT &list) {
        for (auto &c : list) {
            if (std::holds_alternative<NullEvent>(c)) {
                return;
            }

            if (auto key = std::get_if<KeyEvent>(&c)) {
                if (*key == Key::Quit) {
                    quitMedit(mainWindow->env().context());
                    return;
                }

                if (*key != Key::Unknown) {
                    handleKey(c, *mainWindow, *screen);
                }
            }
            else {
                handleKey(c, *mainWindow, *screen);
            }
            mainWindow->triggerRedraw();
        }
    }

    void handleKey(Event e, MainWindow &mainWindow, IScreen &screen) {
        // TODO: Consider moving this function to mainWindow
        if (auto c = std::get_if<KeyEvent>(&e)) {
            mainWindow._env->key(*c);
            mainWindow.keyPress(mainWindow.env().shared_from_this());
            mainWindow.resize();
        }
        if (auto p = std::get_if<PasteEvent>(&e)) {
            mainWindow.paste(p->text);
        }
        if (auto p = std::get_if<ResizeEvent>(&e)) {
            mainWindow.resize(p->width, p->height);
        }
        if (auto ce = std::get_if<MouseDownEvent>(&e)) {
            mainWindow.mouseDown(ce->x, ce->y);
        }
    }
};

struct MainData {
    std::unique_ptr<CoreEnvironment> core;
    std::shared_ptr<IJobQueue> guiQueue;
    std::shared_ptr<IJobQueue> jobQueue;
    std::shared_ptr<ITimer> timer;
    std::shared_ptr<ITimer> guiLoopTimer;
    std::shared_ptr<ThreadContext> context;
    std::unique_ptr<TcpServer> server;

    std::vector<std::unique_ptr<User>> users;

    MainData() = default; // Static initialization

    void start(const Settings &settings);

    void loop();

    void stop();
};

// Its defined publicly so that emscripten can keep it after the main function
// exits
MainData mainData;

void MainData::start(const Settings &settings) {
    jobQueue = std::make_shared<QueueType>();
    guiQueue = std::make_shared<QueueType>();
    timer = std::make_shared<TimerType>();

    context = std::make_shared<ThreadContext>(*jobQueue, *guiQueue, *timer);

    mainData.core = std::make_unique<CoreEnvironment>(*context);

    registerDefaultPlugins(*core);
    core->plugins().sort();

    timer->start();
    jobQueue->start();

    if (settings.style == UiStyle::TcpServer) {
        server = std::make_unique<TcpServer>(settings.port);

        std::cout << "starting tcp server..." << std::endl;
        server->accept([this, settings](std::shared_ptr<IConnection> conn) {
            guiQueue->addTask([this, conn, settings] {
                users.push_back(
                    std::make_unique<User>(conn, *core, settings, *context));
            });
            std::cout << "client connected..." << std::endl;
        });
    }
    else {
        users.push_back(std::make_unique<User>(settings, *core, *context));
    }
}

void MainData::stop() {
    jobQueue->stop();
    timer->stop();
    guiQueue->stop();
}

void MainData::loop() {

#ifdef __EMSCRIPTEN__
    guiQueue->start();
#else
    while (!medit::main::shouldQuit) {
        guiQueue->work(true);
        for (auto &user : users) {
            user->mainWindow->refreshScreen();
        }
    }
#endif
}

} // namespace

int main(int argc, char **argv) {
    auto settings = Settings{argc, argv};

    setThreadName("main");
    setupSignals();

    if (settings.enablePerformanceProfiling) {
        enableProfiling();
    }

#ifndef __EMSCRIPTEN__
    if (settings.style == UiStyle::FifoClient ||
        settings.style == UiStyle::TcpClient) {
        return thinMain(settings);
    }
#endif

    mainData.start(settings);
    setThreadName("main loop"); // Renaming since all child threads inherits the
                                // first name
    mainData.loop();
#ifndef __EMSCRIPTEN__
    mainData.stop();
#endif

    return 0;
}
