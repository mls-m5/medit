
#include "views/mainwindow.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "screen/iscreen.h"

MainWindow::MainWindow(size_t w, size_t h) : View(w, h) {
    env.editor(&editor);
    //        env.editor(&commandBuffer);
    editor.mode(createNormalMode());
    console.showLines(false);
    env.console(&console);
    for (size_t i = 0; i < width(); ++i) {
        splitString.insert(splitString.end(), FChar{'-', 6});
    }
    commandBuffer.mode(createInsertMode());
}

void MainWindow::draw(IScreen &screen) {
    editor.draw(screen);
    if (env.showConsole()) {
        console.draw(screen);
        screen.draw(0, height() - split, splitString);
    }

    if (inputFocus == &commandBuffer) {
        commandBuffer.draw(screen);
    }
}
