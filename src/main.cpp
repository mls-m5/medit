#include "files/file.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "screen/linuxterminalscreen.h"
#include "screen/ncursesscreen.h"
#include "script/environment.h"
#include "views/bufferview.h"
#include "views/editor.h"
#include <string>
#include <vector>

namespace {
bool shouldQuit = false;

struct MainWindow : public View, public IKeySink {
    Editor editor;
    Environment env;
    BufferView console{std::make_unique<Buffer>()};
    Editor commandBuffer;
    size_t split = 10;

    IKeySink *inputFocus = &editor;
    //    IKeySink *inputFocus = &commandBuffer;

    FString splitString;

    MainWindow(size_t w, size_t h) : View(w, h) {
        env.editor(&editor);
        //        env.editor(&commandBuffer);
        editor.mode(createNormalMode());
        console.showLines(false);
        env.console(&console.buffer());
        for (size_t i = 0; i < width(); ++i) {
            splitString.insert(splitString.end(), FChar{'-', 6});
        }
        commandBuffer.mode(createInsertMode());
    }

    ~MainWindow() override = default;

    void resize() {
        //! Todo: Handle layouts better in the future
        editor.width(width());
        if (env.showConsole()) {
            editor.height(height() - split);
        }
        else {
            editor.height(height() - 1);
        }
        editor.x(0);
        editor.y(0);

        console.width(width());
        console.height(split - 1); // 1 character for toolbar
        console.x(0);
        console.y(height() - split + 1);

        commandBuffer.x(0);
        commandBuffer.y(0);
        commandBuffer.width(width());
        commandBuffer.height(1);
    }

    //! @see IView
    void draw(IScreen &screen) override {
        editor.draw(screen);
        if (env.showConsole()) {
            console.draw(screen);
            screen.draw(0, height() - split, splitString);
        }

        if (inputFocus == &commandBuffer) {
            commandBuffer.draw(screen);
        }
    }

    //! @see IKeySink
    void updateCursor(IScreen &screen) const override {
        inputFocus->updateCursor(screen);
    }

    //! @see IKeySink
    void keyPress(IEnvironment &env) override {
        inputFocus->keyPress(env);
    }
};

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
        file->load(mainWindow.editor.buffer());
        mainWindow.editor.file(std::move(file));
    }

    mainWindow.resize();
    mainWindow.draw(*screen);
    mainWindow.updateCursor(*screen);

    while (!shouldQuit) {
        auto c = input->getInput();
        mainWindow.env.key(c);
        screen->clear();
        mainWindow.keyPress(mainWindow.env);
        mainWindow.resize();

        mainWindow.draw(*screen);

        screen->draw(40,
                     screen->height() - 1,
                     std::string{c.symbol} + c.symbol.byteRepresentation());

        mainWindow.updateCursor(*screen);

        screen->refresh();
    }

    return 0;
}
