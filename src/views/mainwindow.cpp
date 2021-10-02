
#include "views/mainwindow.h"
#include "core/context.h"
#include "core/jobqueue.h"
#include "core/plugins.h"
#include "core/timer.h"
#include "files/config.h"
#include "files/file.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "navigation/inavigation.h"
#include "screen/iscreen.h"
#include "syntax/iannotation.h"
#include "syntax/ihighlight.h"
#include "syntax/ipalette.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/inputbox.h"
#include "views/messagebox.h"

MainWindow::MainWindow(IScreen &screen, Context &context)
    : View(screen.width(), screen.height()), _editors(1),
      _env(std::make_shared<RootEnvironment>(context)), _locator(_project),
      _currentEditor(0) {
    _env->editor(&_editors.front());
    for (auto &editor : _editors) {
        editor.showLines(true);
    }
    _console.showLines(false);
    _env->console(&_console);
    _env->project(&_project);
    screen.palette().load(findConfig("data/oblivion.json"));

    _locator.mode(createInsertMode());
    _locator.showLines(false);

    _locator.callback([this](auto &&path) {
        open(path);
        _inputFocus = &currentEditor();
        _locator.visible(false);
    });

    _completeView.visible(false);
    _completeView.callback([this](auto &&result) {
        auto &editor = currentEditor();
        auto cursor = editor.cursor();
        for (auto c : result.value) {
            cursor = insert(c, cursor);
        }
        editor.cursor(cursor);
        _inputFocus = &editor;
    });

    addCommands();

    _highlighting = createHighlightings();
    std::sort(_highlighting.begin(),
              _highlighting.end(),
              [](auto &&a, auto &&b) { return a->priority() > b->priority(); });

    updatePalette(screen);

    _formatting = createFormat();

    _annotation = createAnnotations();

    _navigation = createNavigation();
}

MainWindow::~MainWindow() = default;

void MainWindow::addCommands() {
    _env->addCommand("window.show_locator", [this](auto &&) {
        _locator.visible(true);
        _inputFocus = &_locator;
    });

    _env->addCommand("editor.auto_complete",
                     [this](std::shared_ptr<IEnvironment> env) {
                         auto &editor = env->editor();
                         editor.cursor(fix(editor.cursor()));

                         Cursor cursor = editor.cursor();
                         _completeView.setCursor(cursor, editor.bufferView());
                         _completeView.triggerShow(_env);
                     });

    _env->addCommand("editor.format",
                     [this](std::shared_ptr<IEnvironment> env) {
                         auto &editor = env->editor();
                         for (auto &format : _formatting) {
                             if (format->format(editor)) {
                                 break;
                             }
                         }
                     });

    _env->addCommand("editor.goto_definition",
                     [this](std::shared_ptr<IEnvironment> env) {
                         for (auto &navigation : _navigation) {
                             if (navigation->gotoSymbol(env)) {
                                 break;
                             }
                         }
                     });

    _env->addCommand("editor.open", [this](std::shared_ptr<IEnvironment> env) {
        auto path = env->get("path");
        if (path) {
            open(path->value());
        }
    });

    _env->addCommand("messagebox", [this](auto &&) {
        showPopup(std::make_unique<MessageBox>());
    });

    _env->addCommand(
        "editor.show_open", [this](std::shared_ptr<IEnvironment> env) {
            auto &editor = env->editor();
            auto path = editor.path();
            if (path.empty()) {
                path = filesystem::current_path();
            }
            auto input =
                std::make_unique<InputBox>("Path to open: ", path.string());
            input->callback([this](std::string value) { open(value); });
            showPopup(std::move(input));
        });

    _env->addCommand("show_console", [this](auto env) {
        env->showConsole(true);
        _inputFocus = &_console;
    });

    _env->addCommand("escape", [this](std::shared_ptr<IEnvironment> env) {
        env->showConsole(false);
        _inputFocus = &currentEditor();
    });

    _env->addCommand("switch_editor", [this](auto &&) { switchEditor(); });
}

void MainWindow::resize(size_t w, size_t h) {
    //! Todo: Handle layouts better in the future
    if (w) {
        width(w);
    }
    if (h) {
        height(h);
    }

    {
        size_t index = 0;
        for (auto &editor : _editors) {
            editor.x(index * width() / 2);
            editor.y(0);
            editor.width(width() / 2);
            if (_env->showConsole()) {
                editor.height(height() - _split);
            }
            else {
                editor.height(height() - 1);
            }

            ++index;
        }
    }

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
    screen.cursorStyle(currentEditor().mode().cursorStyle());

    for (auto &editor : _editors) {
        editor.draw(screen);
    }
    if (_env->showConsole()) {
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

    auto c = _env->key();
    screen.draw(40,
                screen.height() - 1,
                ((c.modifiers == Modifiers::Ctrl) ? "ctrl+'" : "'") +
                    c.symbol.toString() + "'" + c.symbol.byteRepresentation());
}

bool MainWindow::keyPress(std::shared_ptr<IEnvironment> env) {
    if (_inputFocus == &currentEditor() && _completeView.visible()) {
        if (_completeView.keyPress(env)) {
            updateHighlighting(currentEditor());
            return true; // Otherwise give key events to editor
        }
    }

    auto &editor = currentEditor();
    _env->editor(&editor);
    auto scopeEnvironment = std::make_shared<Environment>(env);
    scopeEnvironment->editor(&editor);
    if (_inputFocus->keyPress(scopeEnvironment)) {
        // Todo: Handle this for reallz in the future
        if (_inputFocus == &editor) {
            _env->editor(&editor);
        }
        if (_inputFocus == &_console) {
            _env->editor(&editor);
        }

        updateHighlighting(currentEditor());
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
    auto &editor = currentEditor();
    if (editor.file()) {
        _project.updateCache(editor.path());
    }
    else {
        _project.updateCache(filesystem::current_path());
    }
}

void MainWindow::open(filesystem::path path) {
    if (path.empty()) {
        return;
    }
    auto &editor = currentEditor();
    path = filesystem::absolute(path);
    auto file = std::make_unique<File>(path);

    editor.buffer(std::make_unique<Buffer>());
    editor.cursor(Cursor{editor.buffer()});
    if (filesystem::exists(path)) {
        file->load(editor.buffer());
    }
    else {
        editor.buffer().clear();
    }
    editor.file(std::move(file));
    editor.bufferView().yScroll(0);
    updateLocatorBuffer();

    updateHighlighting(editor);
}

void MainWindow::updatePalette(IScreen &screen) {
    if (screen.palette().update(screen)) {
        for (auto &highligting : _highlighting) {
            highligting->update(screen.palette());
        }
    }
}

void MainWindow::updateHighlighting(Editor &editor) {

    auto &timer = _env->context().timer();
    if (_updateTimeHandle) {
        timer.cancel(_updateTimeHandle);
        _updateTimeHandle = 0;
    }

    if (editor.buffer().isColorsOld()) {
        _updateTimeHandle = timer.setTimeout(1s, [this, env = _env] {
            auto &queue = env->context().guiQueue();
            auto &editor = env->editor();

            if (editor.buffer().isColorsOld()) {
                queue.addTask([this, env] {
                    auto &editor = env->editor();
                    for (auto &highlight : _highlighting) {
                        if (highlight->shouldEnable(editor.path())) {
                            highlight->highlight(env);

                            editor.buffer().isColorsOld(false);
                            break;
                        }
                    }

                    for (auto &annotation : _annotation) {
                        if (annotation->shouldEnable(editor.path())) {
                            annotation->annotate(env);
                            break;
                        }
                    }
                    env->context().redrawScreen();
                });
            }
        });
    }
}

void MainWindow::showPopup(std::unique_ptr<IWindow> popup) {
    _activePopup = std::move(popup);
    _inputFocus = _activePopup.get();
}

Editor &MainWindow::currentEditor() {
    if (_currentEditor >= _editors.size()) {
        _currentEditor = _editors.size() - 1;
    }
    return _editors.at(_currentEditor);

    throw std::runtime_error("could not get the right editor");
}

void MainWindow::resetFocus() {
    _inputFocus = &_editors.at(_currentEditor);
}

void MainWindow::switchEditor() {
    _currentEditor = (_currentEditor + 1) % _editors.size();
    _inputFocus = &currentEditor();
}
