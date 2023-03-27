
#include "views/mainwindow.h"
#include "core/context.h"
#include "core/coreenvironment.h"
#include "core/ijobqueue.h"
#include "core/itimer.h"
#include "core/plugins.h"
#include "files/config.h"
#include "modes/insertmode.h"
#include "navigation/inavigation.h"
#include "screen/iscreen.h"
#include "script/localenvironment.h"
#include "syntax/iannotation.h"
#include "syntax/ihighlight.h"
#include "syntax/palette.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/inputbox.h"
#include <memory>

MainWindow::MainWindow(IScreen &screen, Context &context)
    : Window{nullptr}
    , View(nullptr, 100, 100)
    , _screen{screen}
    , _editors{}
    , _env(std::make_unique<LocalEnvironment>(*this, context))
    //    , _scope(std::make_shared<RootScope>(*_env))
    , _console(this, _env->core().create(_env))
    , _locator(this, _project)
    , _completeView(this)
    , _currentEditor(0) {

    _editors.push_back(
        std::make_unique<Editor>(this, _env->core().create(_env)));
    _inputFocus = _editors.front().get();

    //    _scope->editor(_editors.front().get());
    for (auto &editor : _editors) {
        editor->showLines(true);
    }
    _console.showLines(false);
    _env->console(&_console);
    _env->project(&_project);

    {
        auto palette = Palette{};
        palette.load(findConfig("data/oblivion.json"));
        screen.palette(palette);
    }

    _locator.visible(false);
    _locator.mode(createInsertMode());
    _locator.showLines(false);

    _locator.callback([this](auto &&path) {
        _locator.visible(false);
        open(path);
        _inputFocus = currentEditor();
    });

    _completeView.visible(false);
    _completeView.callback([this](auto &&result) {
        auto editor = currentEditor();
        auto cursor = editor->cursor();
        for (auto c : result.value) {
            cursor = insert(c, cursor);
        }
        editor->cursor(cursor);
        _inputFocus = editor;
    });

    //    addCommands(screen);

    _highlighting = createHighlightings();
    std::sort(_highlighting.begin(),
              _highlighting.end(),
              [](auto &&a, auto &&b) { return a->priority() > b->priority(); });

    _formatting = createFormat();

    _annotation = createAnnotations();

    _navigation = createNavigation();

    updateLocatorBuffer();

    _env->initLua(*this);
}

MainWindow::~MainWindow() = default;

// void MainWindow::addCommands(IScreen &screen) {
//     _scope->addCommand("window.show_locator", [this](auto &&) {
//         _locator.visible(true);
//         _inputFocus = &_locator;
//     });

//    _scope->addCommand("editor.auto_complete",
//                       [this](std::shared_ptr<IEnvironment> env) {
//                           auto &editor = env->editor();
//                           editor.cursor(fix(editor.cursor()));

//                           Cursor cursor = editor.cursor();
//                           _completeView.setCursor(cursor,
//                           editor.bufferView());
//                           _completeView.triggerShow(_scope);
//                       });

//    _scope->addCommand("editor.format",
//                       [this](std::shared_ptr<IEnvironment> env) {
//                           auto &editor = env->editor();
//                           for (auto &format : _formatting) {
//                               if (format->format(editor)) {
//                                   break;
//                               }
//                           }
//                       });

//    _scope->addCommand("editor.goto_definition",
//                       [this](std::shared_ptr<IEnvironment> env) {
//                           for (auto &navigation : _navigation) {
//                               if (navigation->gotoSymbol(env)) {
//                                   break;
//                               }
//                           }
//                       });

//    _scope->addCommand("editor.open", [this](std::shared_ptr<IEnvironment>
//    env)
//    {
//        auto path = env->get("path");
//        if (path) {

//            std::optional<int> ox;
//            std::optional<int> oy;

//            if (auto x = env->get("x")) {
//                ox = std::stoi(x->value());
//            }
//            if (auto y = env->get("y")) {
//                oy = std::stoi(y->value());
//            }

//            open(path->value(), ox, oy);
//        }
//    });

//    _scope->addCommand("messagebox", [this](auto &&) {
//        showPopup(std::make_unique<MessageBox>(*this));
//    });

//    _scope->addCommand(
//        "editor.show_open", [this](std::shared_ptr<IEnvironment> env) {
//            auto &editor = env->editor();
//            auto path = editor.path();
//            if (path.empty()) {
//                path = filesystem::current_path();
//            }
//            auto input = std::make_unique<InputBox>(
//                this, "Path to open: ", path.string());
//            input->callback([this](std::string value) { open(value); });
//            showPopup(std::move(input));
//        });

//    _scope->addCommand("window.title",
//                       [&screen, this](std::shared_ptr<IEnvironment> env)
//                       {
//                           if (auto title = env->get("title")) {
//                               screen.title(title->value());
//                           }
//                           if (auto file = currentEditor().file()) {
//                               screen.title(file->path().string() + " -
//                               medit");
//                           }
//                       });

//    _scope->addCommand("show_console", [this](auto scope) {
//        _env->showConsole(true);
//        _inputFocus = &_console;
//    });

//    _scope->addCommand("escape", [this](std::shared_ptr<IEnvironment>
//    scope) {
//        _env->showConsole(false);
//        _inputFocus = &currentEditor();
//    });

//    _scope->addCommand("switch_editor", [this](auto &&) { switchEditor();
//    });
//}

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
    if (auto e = currentEditor()) {
        screen.cursorStyle(e->mode().cursorStyle());
    }

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

bool MainWindow::keyPress(std::shared_ptr<IEnvironment> env) {
#warning "i should really clean up input handling"

    if (_inputFocus == currentEditor() && _completeView.visible()) {
        if (_completeView.keyPress(env)) {
            if (auto e = currentEditor()) {
                updateHighlighting(*e);
            }
            return true; // Otherwise give key events to editor
        }
    }

    auto editor = currentEditor();
    if (_inputFocus->keyPress(env)) {
        if (auto e = currentEditor()) {
            updateHighlighting(*e);
        }
        if (_activePopup && _activePopup->isClosed()) {
            _activePopup = nullptr;
            resetFocus();
        }
        return true;
    }

    return false;
}

void MainWindow::updateLocatorBuffer() {
    //    auto &editor = currentEditor();
    _project.updateCache(filesystem::current_path());
}

void MainWindow::open(filesystem::path path,
                      std::optional<int> x,
                      std::optional<int> y) {
    if (path.empty()) {
        return;
    }

    if (_activePopup) {
        return;
    }

    auto editor = currentEditor();
    if (!editor) {
        return;
    }

    path = filesystem::absolute(path);

    editor->buffer(_env->core().open(path, _env));
    editor->bufferView().yScroll(0);

    {
        auto cur = editor->cursor();
        if (x) {
            cur.x(*x);
        }
        if (y) {
            cur.y(*y);
        }
        editor->cursor(cur);
    }

    updateHighlighting(*editor);

    updateTitle();
    //    _scope->run({"window.title"});
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

    using namespace std::literals;

    if (editor.buffer().isColorsOld()) {
        _updateTimeHandle = timer.setTimeout(1s, [this] {
            auto &queue = _env->context().guiQueue();
            auto &editor = _env->editor();

            queue.addTask([this, &buffer = editor.buffer()] {
                if (buffer.isColorsOld()) {
                    auto &editor = _env->editor();
                    for (auto &highlight : _highlighting) {
                        if (highlight->shouldEnable(editor.path())) {
                            highlight->highlight(_env);

                            editor.buffer().isColorsOld(false);
                            break;
                        }
                    }

                    for (auto &annotation : _annotation) {
                        if (annotation->shouldEnable(editor.path())) {
                            annotation->annotate(_env);
                            break;
                        }
                    }
                    triggerRedraw();
                }
            });
        });
    }
}

void MainWindow::showPopup(std::unique_ptr<IWindow> popup) {
    _activePopup = std::move(popup);
    _inputFocus = _activePopup.get();
}

Editor *MainWindow::currentEditor() {
    if (_activePopup) {
        if (auto e = _activePopup->currentEditor()) {
            return e;
        }
    }
    if (_locator.visible()) {
        return &_locator;
    }
    if (_currentEditor >= _editors.size()) {
        _currentEditor = _editors.size() - 1;
    }
    return _editors.at(_currentEditor).get();

    throw std::runtime_error("could not get the right editor");
}

void MainWindow::resetFocus() {
    _inputFocus = _editors.at(_currentEditor).get();
}

void MainWindow::switchEditor() {
    _currentEditor = (_currentEditor + 1) % _editors.size();
    _inputFocus = currentEditor();
}

void MainWindow::paste(std::string text) {
    if (auto e = currentEditor()) {
        e->cursor(insert(e->cursor(), text));
    }
}

bool MainWindow::mouseDown(int x, int y) {
    if (auto e = currentEditor()) {
        e->mouseDown(x, y);
        return true;
    }
    return false;
}

void MainWindow::copy(bool shouldCut) {
    if (auto e = currentEditor()) {
        auto text = content(e->selection());
        if (shouldCut) {
            erase(e->selection());
            // TODO: selection clearing should be handled by erase
            e->clearSelection();
        }

        _screen.clipboardData(text);
        return;
    }
    _screen.clipboardData("");
}

void MainWindow::triggerRedraw() {
    _shouldRedraw = true;
    _env->context().guiQueue().addTask([this] { /*refreshScreen();*/ });
}

void MainWindow::refreshScreen() {
    if (_shouldRedraw) {
        _shouldRedraw = false;

        {
            constexpr auto debug = true;
            if (debug) {
                if (auto e = currentEditor()) {
                    auto cursor = e->cursor();
                    _screen.draw(1,
                                 height() - 1,
                                 std::to_string(cursor.y() + 1) + ", " +
                                     std::to_string(cursor.x() + 1));

                    //                _screen.draw(10, height() - 1,
                    //                _mode->name());
                }
            }
        }

        updateCursor(_screen);
        _screen.clear();
        draw(_screen);
        _screen.refresh();
    }
}

void MainWindow::updateTitle() {
    auto editor = currentEditor();
    if (!currentEditor()) {
        return;
    }

    if (auto file = editor->file()) {
        _screen.title(file->path().string() + " - medit");
    }
}

void MainWindow::escape() {
    _env->showConsole(false);
    _inputFocus = currentEditor();
}

void MainWindow::showConsole() {
    _env->showConsole(true);
    _inputFocus = &_console;
}

void MainWindow::showOpen() {
    auto editor = currentEditor();
    if (!editor) {
        return;
    }
    auto path = editor->path();
    if (path.empty()) {
        path = filesystem::current_path();
    }
    auto input =
        std::make_unique<InputBox>(this, "Path to open: ", path.string());
    input->callback([this](std::string value) { open(value); });
    showPopup(std::move(input));
}

void MainWindow::gotoDefinition() {
    for (auto &navigation : _navigation) {
        if (navigation->gotoSymbol(_env)) {
            break;
        }
    }
}

void MainWindow::format() {
    auto &editor = _env->editor();
    for (auto &format : _formatting) {
        if (format->format(editor)) {
            break;
        }
    }
}

void MainWindow::autoComplete() {
    if (_activePopup) {
        return;
    }

    if (auto editor = currentEditor()) {
        editor->cursor(fix(editor->cursor()));

        Cursor cursor = editor->cursor();
        _completeView.setCursor(cursor, editor->bufferView());
        _completeView.triggerShow(_env);
    }
}

void MainWindow::showLocator() {
    _locator.visible(true);
    _inputFocus = &_locator;
}
