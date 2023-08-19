
#include "views/mainwindow.h"
#include "completion/icompletionsource.h"
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
#include "syntax/iformat.h"
#include "syntax/palette.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "text/fstring.h"
#include "text/fstringview.h"
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
    , _console(this, _env->core().files().create())
    , _locator(this, _project)
    , _commandPalette(this, StandardCommands::get())
    , _completeView(
          this, CoreEnvironment::instance().plugins().get<ICompletionSource>())
    , _project{_env->core().files().directoryNotifications()}
    , _currentEditor(0) {

    //    for (int i = 0; i < 2; ++i) {
    _editors.push_back(
        std::make_unique<Editor>(this, _env->core().files().create()));
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
        _inputFocus = currentEditor();
        if (path.empty()) {
            return;
        }
        open(_project.settings().root / path);
    });

    _commandPalette.visible(false);
    _commandPalette.mode(createInsertMode());
    _commandPalette.showLines(false);
    _commandPalette.callback([this](auto &&path) {
        _commandPalette.visible(false);
        if (!path.empty()) {
            StandardCommands::get().namedCommands.at(path)(_env);
        }
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
    _console.height(_split -
                    2); // 1 character for toolbar - 2 for how numbers works
    _console.x(0);
    _console.y(height() - _split + 1);

    _locator.x(0);
    _locator.y(0);
    _locator.width(width());
    _locator.height(1);

    _commandPalette.x(0);
    _commandPalette.y(0);
    _commandPalette.width(width());
    _commandPalette.height(1);

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

    if (_inputFocus == &_commandPalette) {
        _commandPalette.draw(screen);
    }

    _completeView.draw(screen);

    if (_activePopup) {
        _activePopup->draw(screen);
    }

    {
        auto &editor = _editors.at(_currentEditor);
        auto statusMessage =
            FString{" - " + std::string{editor->mode().name()} + " - "} +
            _statusMessage;
        auto fill = FString{width(), FChar{" "}};
        _screen.draw(0, height() - 1, fill);
        _screen.draw(0, height() - 1, statusMessage);

        auto cur = editor->cursor();
        auto cursorMessage = FString{std::to_string(cur.y() + 1) + ", " +
                                     std::to_string(cur.x() + 1)};

        _screen.draw(
            width() - cursorMessage.size() - 1, height() - 1, cursorMessage);
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
    _project.updateCache(std::filesystem::current_path());
    _env->core().files().directoryNotifications().path(
        _project.settings()
            .root); // TODO: This should probably be handled somewhere else
}

void MainWindow::open(std::filesystem::path path,
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

    path = std::filesystem::absolute(path);

    if (!std::filesystem::is_regular_file(path)) {
        statusMessage(
            FString{"trying to open non existing path " + path.string()});
        return;
    }

    editor->buffer(_env->core().files().open(path));

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
    // Create some more generic layout management for this
    if (_activePopup) {
        if (auto e = _activePopup->currentEditor()) {
            return e;
        }
    }
    if (_locator.visible()) {
        return &_locator;
    }
    if (_commandPalette.visible()) {
        return &_commandPalette;
    }
    if (_env->showConsole() && _inputFocus == &_console) {
        return &_console;
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
        e->selection(replace(e->selection(), text));
        StandardCommands::get().normal_mode(env().shared_from_this());
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
        _screen.title("[unnamed buffer]*");
    }
}

void MainWindow::statusMessage(FStringView str) {
    using namespace std::chrono_literals;
    _statusMessage = FString{str};

    auto &timer = _env->core().context().timer();
    timer.cancel(_statusTimerHandle);

    _statusTimerHandle = timer.setTimeout(5s, [this] {
        _statusMessage = {};
        triggerRedraw();
    });

    triggerRedraw();
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
        path = std::filesystem::current_path();
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

void MainWindow::showCommandPalette() {
    _commandPalette.visible(true);
    _inputFocus = &_commandPalette;
}
