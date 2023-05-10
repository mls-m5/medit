
#include "views/mainwindow.h"
#include "completion/icompletionsource.h"
#include "core/context.h"
#include "core/coreenvironment.h"
#include "core/ijobqueue.h"
#include "core/itimer.h"
#include "core/plugins.h"
#include "files/config.h"
#include "linux/inotify.h"
#include "meditfwd.h"
#include "modes/insertmode.h"
#include "navigation/inavigation.h"
#include "screen/iscreen.h"
#include "script/localenvironment.h"
#include "syntax/iformat.h"
#include "syntax/palette.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/inputbox.h"
#include <filesystem>
#include <memory>

MainWindow::MainWindow(IScreen &screen, ThreadContext &context)
    : Window{nullptr}
    , View(nullptr, 100, 100)
    , _screen{screen}
    , _editors{}
    , _interactions{*this}
    , _env(std::make_unique<LocalEnvironment>(*this, context))
    , _console(this, _env->core().files().create(_env))
    , _locator(this, _project)
    , _completeView(
          this, CoreEnvironment::instance().plugins().get<ICompletionSource>())
    , _currentEditor(0) {

    //    for (int i = 0; i < 2; ++i) {
    _editors.push_back(
        std::make_unique<Editor>(this, _env->core().files().create(_env)));
    //    }
    _inputFocus = _editors.front().get();

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

    updateLocatorBuffer();
}

MainWindow::~MainWindow() = default;

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
        auto w = width() / _editors.size();
        for (auto &editor : _editors) {
            editor->x(index * w);
            editor->y(0);
            editor->width(w);
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
    if (_inputFocus == currentEditor() && _completeView.visible()) {
        if (_completeView.keyPress(env)) {
            if (auto e = currentEditor()) {
                env->core().files().updateHighlighting();
                //                updateHighlighting(e->buffer());
            }
            return true; // Otherwise give key events to editor
        }
    }

    /// In practice only check if the key if return or escape
    if (_interactions.keyPress(env)) {
        return true;
    }

    if (_inputFocus->keyPress(env)) {
        if (auto e = currentEditor()) {
            env->core().files().updateHighlighting();
            //            updateHighlighting(e->buffer());
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
    _project.updateCache(filesystem::current_path());
    _env->core().files().directoryNotifications().path(
        _project.settings()
            .root); // TODO: This should probably be handled somewhere else
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

    editor->buffer(_env->core().files().open(path, _env));

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

    _env->core().files().updateHighlighting();

    updateTitle();
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
    if (!_shouldRedraw) {
        _shouldRedraw = true;
        _env->context().guiQueue().addTask([this] { /*refreshScreen();*/ });
    }
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
                                 FString{std::to_string(cursor.y() + 1) + ", " +
                                         std::to_string(cursor.x() + 1)});
                }
            }
        }

        updateTitle();
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
        auto path =
            std::filesystem::relative(file->path(), _project.settings().root);
        auto title = path.string() + " - medit";
        if (editor->buffer().isChanged()) {
            title += "*";
        }
        _screen.title(title);
    }
    else {
        _screen.title("unnamed*");
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
    for (auto &navigation : _env->core().plugins().get<INavigation>()) {
        if (navigation->gotoSymbol(_env)) {
            break;
        }
    }
}

void MainWindow::format() {
    auto &editor = _env->editor();
    for (auto &format : _env->core().plugins().get<IFormat>()) {
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
        _completeView.setCursor(cursor, *editor);
        _completeView.triggerShow(_env);
    }
}

void MainWindow::showLocator() {
    _locator.visible(true);
    _inputFocus = &_locator;
}
