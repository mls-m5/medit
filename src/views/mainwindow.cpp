
#include "views/mainwindow.h"
#include "core/context.h"
#include "core/coreenvironment.h"
#include "core/ijobqueue.h"
#include "core/plugins.h"
#include "core/timer.h"
#include "files/config.h"
#include "files/file.h"
#include "modes/insertmode.h"
#include "navigation/inavigation.h"
#include "screen/iscreen.h"
#include "script/localenvironment.h"
#include "syntax/iannotation.h"
#include "syntax/ihighlight.h"
#include "syntax/ipalette.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/inputbox.h"
#include "views/messagebox.h"
#include <memory>

MainWindow::MainWindow(IScreen &screen, Context &context)
    : View(screen.width(), screen.height())
    , _editors{}
    , _env(std::make_unique<LocalEnvironment>(context))
    , _scope(std::make_shared<RootScope>(*_env))
    , _console(_env->core().create(_env))
    , _locator(_project)
    , _currentEditor(0) {

    _editors.push_back(std::make_unique<Editor>(_env->core().create(_env)));
    _inputFocus = _editors.front().get();

    _scope->editor(_editors.front().get());
    for (auto &editor : _editors) {
        editor->showLines(true);
    }
    _console.showLines(false);
    _env->console(&_console);
    _env->project(&_project);
    //    screen.palette().load(findConfig("data/oblivion.json"));

    {
        auto palette = Palette{};
        palette.load(findConfig("data/oblivion.json"));
        screen.palette(palette);
    }

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

    addCommands(screen);

    _highlighting = createHighlightings();
    std::sort(_highlighting.begin(),
              _highlighting.end(),
              [](auto &&a, auto &&b) { return a->priority() > b->priority(); });

    //    updatePalette(screen);

    _formatting = createFormat();

    _annotation = createAnnotations();

    _navigation = createNavigation();

    updateLocatorBuffer();
}

MainWindow::~MainWindow() = default;

void MainWindow::addCommands(IScreen &screen) {
    _scope->addCommand("window.show_locator", [this](auto &&) {
        _locator.visible(true);
        _inputFocus = &_locator;
    });

    _scope->addCommand("editor.auto_complete",
                       [this](std::shared_ptr<IScope> env) {
                           auto &editor = env->editor();
                           editor.cursor(fix(editor.cursor()));

                           Cursor cursor = editor.cursor();
                           _completeView.setCursor(cursor, editor.bufferView());
                           _completeView.triggerShow(_scope);
                       });

    _scope->addCommand("editor.format", [this](std::shared_ptr<IScope> env) {
        auto &editor = env->editor();
        for (auto &format : _formatting) {
            if (format->format(editor)) {
                break;
            }
        }
    });

    _scope->addCommand("editor.goto_definition",
                       [this](std::shared_ptr<IScope> env) {
                           for (auto &navigation : _navigation) {
                               if (navigation->gotoSymbol(env)) {
                                   break;
                               }
                           }
                       });

    _scope->addCommand("editor.open", [this](std::shared_ptr<IScope> env) {
        auto path = env->get("path");
        if (path) {

            std::optional<int> ox;
            std::optional<int> oy;

            if (auto x = env->get("x")) {
                ox = std::stoi(x->value());
            }
            if (auto y = env->get("y")) {
                oy = std::stoi(y->value());
            }

            open(path->value(), ox, oy);
        }
    });

    _scope->addCommand("messagebox", [this](auto &&) {
        showPopup(std::make_unique<MessageBox>());
    });

    _scope->addCommand("editor.show_open", [this](std::shared_ptr<IScope> env) {
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

    _scope->addCommand("window.title",
                       [&screen, this](std::shared_ptr<IScope> env) {
                           if (auto title = env->get("title")) {
                               screen.title(title->value());
                           }
                           if (auto file = currentEditor().file()) {
                               screen.title(file->path().string() + " - medit");
                           }
                       });

    _scope->addCommand("show_console", [this](auto scope) {
        _env->showConsole(true);
        _inputFocus = &_console;
    });

    _scope->addCommand("escape", [this](std::shared_ptr<IScope> scope) {
        _env->showConsole(false);
        _inputFocus = &currentEditor();
    });

    _scope->addCommand("switch_editor", [this](auto &&) { switchEditor(); });
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
            editor->x(index * width() / 2);
            editor->y(0);
            editor->width(width() / 2);
            if (_env->showConsole()) {
                editor->height(height() - _split);
            }
            else {
                editor->height(height() - 1);
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
        editor->draw(screen);
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
}

bool MainWindow::keyPress(std::shared_ptr<IScope> env) {
    if (_inputFocus == &currentEditor() && _completeView.visible()) {
        if (_completeView.keyPress(env)) {
            updateHighlighting(currentEditor());
            return true; // Otherwise give key events to editor
        }
    }

    auto &editor = currentEditor();
    _scope->editor(&editor);
    auto scopeEnvironment = std::make_shared<Scope>(env);
    scopeEnvironment->editor(&editor);
    if (_inputFocus->keyPress(scopeEnvironment)) {
        // Todo: Handle this for reallz in the future
        if (_inputFocus == &editor) {
            _scope->editor(&editor);
        }
        if (_inputFocus == &_console) {
            _scope->editor(&editor);
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
    _project.updateCache(filesystem::current_path());
}

void MainWindow::open(filesystem::path path,
                      std::optional<int> x,
                      std::optional<int> y) {
    if (path.empty()) {
        return;
    }
    auto &editor = currentEditor();
    path = filesystem::absolute(path);

    editor.buffer(_env->core().open(path, _env));
    editor.bufferView().yScroll(0);

    {
        auto cur = editor.cursor();
        if (x) {
            cur.x(*x);
        }
        if (y) {
            cur.y(*y);
        }
        editor.cursor(cur);
    }
    //    updateLocatorBuffer();

    updateHighlighting(editor);

    _env->context().redrawScreen();

    _scope->run({"window.title"});
}

void MainWindow::updatePalette(IScreen &screen) {
    //    if (screen.palette().update(screen)) {
    for (auto &highligting : _highlighting) {
        highligting->update(screen.palette());
    }
    //    }
}

void MainWindow::updateHighlighting(Editor &editor) {
#ifdef __EMSCRIPTEN__
    return; // Todo: find reason for crash
#endif      //__EMSCRIPTEN__
    auto &timer = _env->context().timer();
    if (_updateTimeHandle) {
        timer.cancel(_updateTimeHandle);
        _updateTimeHandle = 0;
    }

    if (editor.buffer().isColorsOld()) {
        _updateTimeHandle =
            timer.setTimeout(1s, [this, env = _env, scope = _scope] {
                auto &queue = _env->context().guiQueue();
                auto &editor = _scope->editor();

                queue.addTask(
                    [this, scope = _scope, &buffer = editor.buffer()] {
                        if (buffer.isColorsOld()) {
                            auto &editor = scope->editor();
                            for (auto &highlight : _highlighting) {
                                if (highlight->shouldEnable(editor.path())) {
                                    highlight->highlight(scope);

                                    editor.buffer().isColorsOld(false);
                                    break;
                                }
                            }

                            for (auto &annotation : _annotation) {
                                if (annotation->shouldEnable(editor.path())) {
                                    annotation->annotate(scope);
                                    break;
                                }
                            }
                            _env->context().redrawScreen();
                        }
                    });
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
    return *_editors.at(_currentEditor);

    throw std::runtime_error("could not get the right editor");
}

void MainWindow::resetFocus() {
    _inputFocus = _editors.at(_currentEditor).get();
}

void MainWindow::switchEditor() {
    _currentEditor = (_currentEditor + 1) % _editors.size();
    _inputFocus = &currentEditor();
}

void MainWindow::paste(std::string text) {
    currentEditor().cursor(insert(currentEditor().cursor(), text));
}

bool MainWindow::mouseDown(int x, int y) {
    currentEditor().mouseDown(x, y);

    return true;
}

std::string MainWindow::copy(bool shouldCut) {
    auto text = content(currentEditor().selection());
    if (shouldCut) {
        erase(currentEditor().selection());
        // TODO: selection clearing should be handled by erase
        currentEditor().clearSelection();
    }
    return text;
}
