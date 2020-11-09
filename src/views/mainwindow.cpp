
#include "views/mainwindow.h"
#include "files/file.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "screen/iscreen.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"

MainWindow::MainWindow(size_t w, size_t h)
    : View(w, h), _locator(_env, _project) {
    _env.editor(&_editor);
    _editor.mode(createNormalMode());
    _console.showLines(false);
    _env.console(&_console);
    for (size_t i = 0; i < width(); ++i) {
        splitString.insert(splitString.end(), FChar{'-', 6});
    }
    _locator.mode(createInsertMode());
    _locator.showLines(false);

    _locator.callback([this](auto &&path) {
        open(path);
        _inputFocus = &_editor;
        _locator.visible(false);
    });

    _completeView.callback([this](auto &&result) {
        auto cursor = _editor.cursor();
        for (auto c : result.value) {
            cursor = insert(c, cursor);
        }
        _editor.cursor(cursor);
        _inputFocus = &_editor;
    });

    addCommands();
}

void MainWindow::addCommands() {
    _env.addCommand("window.show_locator", [this](auto &&) {
        _locator.visible(true);
        _inputFocus = &_locator;
    });

    _env.addCommand("editor.auto_complete", [this](auto &&) {
        _editor.cursor(fix(_editor.cursor()));

        Cursor cursor = _editor.cursor();
        auto currentChar = content(left(cursor)).at(0);
        if (!isspace(currentChar)) {
            // If on for example a newline
            cursor = wordBegin(_editor.cursor());
        }
        auto range = CursorRange(cursor, _editor.cursor());
        _completeView.currentText(content(range).front());
        _completeView.triggerShow(_editor.bufferView().cursorPosition(cursor),
                                  _env);
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

    _locator.x(0);
    _locator.y(0);
    _locator.width(width());
    _locator.height(1);
}

void MainWindow::draw(IScreen &screen) {
    _editor.draw(screen);
    if (_env.showConsole()) {
        _console.draw(screen);
        screen.draw(0, height() - _split, splitString);
    }

    if (_inputFocus == &_locator) {
        _locator.draw(screen);
    }

    _completeView.draw(screen);

    //    _testList.draw(screen);
}

void MainWindow::updateCursor(IScreen &screen) const {
    _inputFocus->updateCursor(screen);
}

bool MainWindow::keyPress(IEnvironment &env) {
    if (_inputFocus == &_editor && _completeView.visible()) {
        if (_completeView.keyPress(env)) {
            return true; // Otherwise give key events to editor
        }
    }
    return _inputFocus->keyPress(env);
}

void MainWindow::updateLocatorBuffer() {
    if (_editor.file()) {
        _project.updateCache(_editor.file()->path());
    }
    else {
        _project.updateCache(filesystem::current_path());
    }
}

void MainWindow::open(filesystem::path path) {
    auto file = std::make_unique<File>(path);
    _editor.cursor(Cursor{_editor.buffer()});
    file->load(_editor.buffer());
    _editor.file(std::move(file));
    _editor.bufferView().yScroll(0);
    updateLocatorBuffer();
}
