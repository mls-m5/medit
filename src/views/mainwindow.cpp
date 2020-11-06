
#include "views/mainwindow.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "screen/iscreen.h"

MainWindow::MainWindow(size_t w, size_t h) : View(w, h), _commandBuffer(_env) {
    _env.editor(&_editor);
    _editor.mode(createNormalMode());
    _console.showLines(false);
    _env.console(&_console);
    for (size_t i = 0; i < width(); ++i) {
        splitString.insert(splitString.end(), FChar{'-', 6});
    }
    _commandBuffer.mode(createInsertMode());
    _commandBuffer.showLines(false);

    _testList.addLine("hello");
    _testList.addLine("there");

    _testList.x(2);
    _testList.y(3);
    _testList.width(20);
    _testList.height(20);
    _testList.callBack([this](auto &&text, auto &&) {
        _env.showConsole(true); //
        _env.console().buffer().push_back(text);
        _inputFocus = &_editor;
        _testList.visible(false);
    });

    _env.addCommand("window.show_locator", [this](auto &&) {
        _testList.visible(true);
        _inputFocus = &_testList;
    });
}

void MainWindow::resize() {
    //! Todo: Handle layouts better in the future
    _editor.width(width());
    if (_env.showConsole()) {
        _editor.height(height() - _split);
    }
    else {
        _editor.height(height() - 1);
    }
    _editor.x(0);
    _editor.y(0);

    _console.width(width());
    _console.height(_split - 1); // 1 character for toolbar
    _console.x(0);
    _console.y(height() - _split + 1);

    _commandBuffer.x(0);
    _commandBuffer.y(0);
    _commandBuffer.width(width());
    _commandBuffer.height(1);
}

void MainWindow::draw(IScreen &screen) {
    _editor.draw(screen);
    if (_env.showConsole()) {
        _console.draw(screen);
        screen.draw(0, height() - _split, splitString);
    }

    if (_inputFocus == &_commandBuffer) {
        _commandBuffer.draw(screen);
    }

    _testList.draw(screen);
}

void MainWindow::updateCursor(IScreen &screen) const {
    _inputFocus->updateCursor(screen);
}

bool MainWindow::keyPress(IEnvironment &env) {
    return _inputFocus->keyPress(env);
}
