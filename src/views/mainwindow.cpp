
#include "views/mainwindow.h"
#include "files/config.h"
#include "files/file.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "plugin/clangformat.h"
#include "plugin/jsonformat.h"
#include "screen/iscreen.h"
#include "syntax/basichighligting.h"
#include "syntax/ipalette.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/messagebox.h"
#include "clang/clanghighlight.h"

MainWindow::MainWindow(IScreen &screen)
    : View(screen.width(), screen.height()), _locator(_env, _project) {
    _env.editor(&_editor);
    _editor.mode(createNormalMode());
    _console.showLines(false);
    _env.console(&_console);
    _env.project(&_project);
    //    _env.palette().load(findConfig("data/oblivion.json"));
    screen.palette().load(findConfig("data/oblivion.json"));

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

    //    _highlighting.push_back(std::make_unique<ClangHighlight>());
    _highlighting.push_back(std::make_unique<BasicHighlighting>());
    updatePalette(screen);

    _formatting.push_back(std::make_unique<ClangFormat>());
    _formatting.push_back(std::make_unique<JsonFormat>());
}

MainWindow::~MainWindow() = default;

void MainWindow::addCommands() {
    _env.addCommand("window.show_locator", [this](auto &&) {
        _locator.visible(true);
        _inputFocus = &_locator;
    });

    _env.addCommand("editor.auto_complete", [this](auto &&) {
        _editor.cursor(fix(_editor.cursor()));

        Cursor cursor = _editor.cursor();
        auto currentChar = content(left(cursor)).at(0);
        if (isalnum(currentChar)) {
            // If on for example a newline
            cursor = wordBegin(_editor.cursor());
        }
        else {
            cursor = _editor.cursor(); // I.e. Empty string
        }
        auto range = CursorRange(cursor, _editor.cursor());
        _completeView.currentText(content(range).front());
        _completeView.triggerShow(_editor.bufferView().cursorPosition(cursor),
                                  _env);
    });

    _env.addCommand("editor.format", [this](auto &&) {
        for (auto &format : _formatting) {
            format->format(_editor);
        }
    });

    _env.addCommand("messagebox", [this](auto &&) {
        showPopup(std::make_unique<MessageBox>());
    });
}

void MainWindow::resize(size_t w, size_t h) {
    //! Todo: Handle layouts better in the future
    if (w) {
        width(w);
    }
    if (h) {
        height(h);
    }
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

    _splitString = FString{width(), FChar{'-', 6}};
}

void MainWindow::draw(IScreen &screen) {
    updatePalette(screen);

    _editor.draw(screen);
    if (_env.showConsole()) {
        _console.draw(screen);
        screen.draw(0, height() - _split, _splitString);
    }

    if (_inputFocus == &_locator) {
        _locator.draw(screen);
    }

    _completeView.draw(screen);

    if (_activePopup) {
        _activePopup->draw(screen);
    }
}

void MainWindow::updateCursor(IScreen &screen) const {
    _inputFocus->updateCursor(screen);
}

bool MainWindow::keyPress(IEnvironment &env) {
    if (_inputFocus == &_editor && _completeView.visible()) {
        if (_completeView.keyPress(env)) {
            updateHighlighting();
            return true; // Otherwise give key events to editor
        }
    }

    if (_inputFocus->keyPress(env)) {
        updateHighlighting();
        if (_activePopup && _activePopup->isClosed()) {
            _activePopup = nullptr;
            resetFocus();
        }
        return true;
    }
    else {
        return false;
    }
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

    updateHighlighting();
}

void MainWindow::updatePalette(IScreen &screen) {
    if (screen.palette().update(screen)) {
        for (auto &highligting : _highlighting) {
            highligting->update(screen.palette());
        }
        _editor.updatePalette(screen.palette());
    }
}

void MainWindow::updateHighlighting() {
    if (_editor.buffer().oldColors()) {
        for (auto &highlight : _highlighting) {
            if (highlight->shouldEnable(_editor.path())) {
                highlight->highlight(_editor);

                _editor.buffer().oldColors(false);
                break;
            }
        }
    }
}

void MainWindow::showPopup(std::unique_ptr<IWindow> popup) {
    _activePopup = std::move(popup);
    _inputFocus = _activePopup.get();
}

void MainWindow::resetFocus() {
    _inputFocus = &_editor;
}
