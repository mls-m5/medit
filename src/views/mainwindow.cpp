
#include "views/mainwindow.h"
#include "completion/icompletionsource.h"
#include "core/context.h"
#include "core/coreenvironment.h"
#include "core/ijobqueue.h"
#include "core/itimer.h"
#include "core/logtype.h"
#include "core/plugins.h"
#include "files/config.h"
#include "files/unsavablefile.h"
#include "modes/insertmode.h"
#include "navigation/inavigation.h"
#include "screen/iscreen.h"
#include "script/localenvironment.h"
#include "script/staticcommandregister.h"
#include "syntax/basichighligting.h"
#include "syntax/iformat.h"
#include "syntax/palette.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "text/fstring.h"
#include "text/fstringview.h"
#include "views/commandpalette.h"
#include "views/completeview.h"
#include "views/editor.h"
#include "views/locator.h"
#include <filesystem>
#include <memory>
#include <string_view>

namespace {

StaticCommandRegister MainWindowReg{{
    {"command_palette",
     [](auto &&env) { env->mainWindow().showCommandPalette(); }},
    {"split_editor", [](auto &&env) { env->mainWindow().splitEditor(); }},
    {"close_editor", [](auto &&env) { env->mainWindow().closeEditor(); }},
    {"cut", [](auto &&env) { env->mainWindow().copy(true); }},
    {"copy", [](auto &&env) { env->mainWindow().copy(false); }},
    {"escape", [](auto &&env) { env->mainWindow().escape(); }},
}};

}

MainWindow::MainWindow(CoreEnvironment &core,
                       IScreen &screen,
                       ThreadContext &context)
    : Window{nullptr}
    , View(nullptr, 100, 100)
    , _screen{screen}
    , _editors{}
    , _interactions{*this}
    , _env(std::make_unique<LocalEnvironment>(core, *this, context))
    , _console(std::make_shared<Editor>(this, _env->core().files().create()))
    , _locator(std::make_shared<Locator>(this, core.project()))
    , _commandPalette(
          std::make_shared<CommandPalette>(this, StandardCommands::get()))
    , _completeView(std::make_shared<CompleteView>(
          this, core.plugins().get<ICompletionSource>()))
    , _currentEditor(0) {

    for (int i = 0; i < 1; ++i) {
        _editors.push_back(
            std::make_unique<Editor>(this, _env->core().files().create()));
    }
    _inputFocus = _editors.front().get();

    for (auto &editor : _editors) {
        editor->showLines(true);
    }
    _console->showLines(false);
    _console->buffer().assignFile(std::make_unique<UnsavableFile>("Console"));
    _env->console(_console.get());
    _env->core().subscribeToLogCallback(
        [this](LogType type, std::string data) {
            _env->context().guiQueue().addTask(
                [this, data = std::move(data), type] {
                    if (type == LogType::StatusMessage) {
                        _env->statusMessage(FString{data});
                    }
                    else {
                        showConsole();
                        // TODO: Create some nice formatting for errors and add
                        // that. The current is unusable
                        _console->buffer().pushBack(FString{data});
                        _console->cursor(
                            Cursor(_console->buffer(),
                                   0,
                                   _console->buffer().lines().size() - 1));
                    }
                });
        },
        this);

    {
        auto palette = Palette{};
        palette.load(findConfig("data/oblivion.json"));
        screen.palette(palette);
    }

    _locator->visible(false);
    _locator->mode(createInsertMode());
    _locator->showLines(false);

    _locator->callback([this](auto &&path) {
        _locator->visible(false);
        _inputFocus = currentEditor();
        if (path.empty()) {
            return;
        }
        open(env().project().settings().root / path);
    });

    _commandPalette->visible(false);
    _commandPalette->mode(createInsertMode());
    _commandPalette->showLines(false);
    _commandPalette->callback([this](auto &&path) {
        _commandPalette->visible(false);
        if (!path.empty()) {
            StandardCommands::get().namedCommands.at(path).f(_env);
        }
        _inputFocus = currentEditor();
    });

    _completeView->visible(false);
    _completeView->callback([this](auto &&result) {
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

MainWindow::~MainWindow() {
    _env->core().unsubscribeToConsoleCallback(this);
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

    _console->width(width());
    _console->height(_split -
                     2); // 1 character for toolbar - 2 for how numbers works
    _console->x(0);
    _console->y(height() - _split + 1);

    _locator->x(0);
    _locator->y(0);
    _locator->width(width());
    _locator->height(1);

    _commandPalette->x(0);
    _commandPalette->y(0);
    _commandPalette->width(width());
    _commandPalette->height(1);

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
        _console->draw(screen);
        screen.draw(0, height() - _split, _splitString);
    }

    if (_inputFocus == _locator.get()) {
        _locator->draw(screen);
    }

    if (_inputFocus == _commandPalette.get()) {
        _commandPalette->draw(screen);
    }

    _completeView->draw(screen);

    {
        auto &editor = _editors.at(_currentEditor);
        auto statusMessage =
            FString{" - " + std::string{editor->mode().name()} + " - "} +
            _statusMessage;
        auto fill = FString{width(), FChar{" "}};
        _screen.draw(0, height() - 1, fill);
        _screen.draw(0, height() - 1, statusMessage);

        auto cur = editor->cursor();
        auto cursorMessage =
            FString{std::to_string(cur.y() + 1) + "/" +
                    std::to_string(cur.buffer().lines().size()) + ", " +
                    std::to_string(cur.x() + 1)};

        _screen.draw(
            width() - cursorMessage.size() - 1, height() - 1, cursorMessage);
    }
}

void MainWindow::updateCursor(IScreen &screen) const {
    _inputFocus->updateCursor(screen);
}

bool MainWindow::keyPress(std::shared_ptr<IEnvironment> env) {
    if (_inputFocus == currentEditor() && _completeView->visible()) {
        if (_completeView->keyPress(env)) {
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
            _jumpList.updatePosition(e->shared_from_this());
        }
        return true;
    }

    return false;
}

void MainWindow::updateLocatorBuffer() {
    auto &project = _env->core().project();
    project.updateCache(std::filesystem::current_path());
    _env->core().files().directoryNotifications().path(
        project.settings()
            .root); // TODO: This should probably be handled somewhere else
}

bool MainWindow::open(std::filesystem::path path,
                      std::optional<int> x,
                      std::optional<int> y) {
    if (path.empty()) {
        return false;
    }

    auto editor = currentEditor();
    if (!editor) {
        return false;
    }

    path = std::filesystem::absolute(path);

    if (!std::filesystem::is_regular_file(path)) {
        statusMessage(
            FString{"trying to open non existing path " + path.string()});
        return false;
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

    // Do a basic (and fast) highlighting before the lsp highligting kicks in
    BasicHighlighting::highlightStatic(editor->buffer());

    updateTitle();

    _jumpList.updatePosition(editor->shared_from_this());
    return true;
}

Editor *MainWindow::currentEditor() {
    if (_locator->visible()) {
        return _locator.get();
    }
    if (_commandPalette->visible()) {
        return _commandPalette.get();
    }
    if (_env->showConsole() && _inputFocus == _console.get()) {
        return _console.get();
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
        _env->standardCommands().normal_mode(env().shared_from_this());
        e->selection(replace(e->selection(), text));
        e->clearSelection();
    }
}

bool MainWindow::mouseDown(int x, int y) {
    for (size_t i = 0; i < _editors.size(); ++i) {
        auto &e = *_editors.at(i);
        if (e.mouseDown(x, y)) {
            _currentEditor = i;
            _inputFocus = &e;
            return true;
        }
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
        if (shouldCut) {
            _env->statusMessage({"cut to clipboard"});
        }
        else {
            _env->statusMessage({"copied to clipboard"});
        }
        return;
    }
    _screen.clipboardData("");
    _env->statusMessage({"failed to copy to clipboard"});
}

void MainWindow::triggerRedraw() {
    if (!_shouldRedraw) {
        _shouldRedraw = true;
        _env->context().guiQueue().addTask([] {}); // Automatically redraws
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
        _screen.clear();
        draw(_screen);
        updateCursor(_screen);
        _screen.refresh();
    }
}

void MainWindow::updateTitle() {
    auto editor = currentEditor();
    if (!currentEditor()) {
        return;
    }

    if (auto file = editor->file()) {
        auto path = file->path();
        if (!path.empty() && *path.begin() == "tmp:") {
            // Do nothing
        }
        else if (path.has_parent_path()) {
            path = std::filesystem::relative(
                file->path(), _env->core().project().settings().root);
        }
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
    _inputFocus = _console.get();
}

void MainWindow::splitEditor() {
    auto &buffer = _editors.back()->buffer();
    _editors.push_back(
        std::make_unique<Editor>(this, buffer.shared_from_this()));
    _editors.back()->showLines(true);
}

void MainWindow::closeEditor() {
    if (_editors.size() <= 1) {
        return;
    }

    _editors.erase(_editors.begin() + _currentEditor,
                   _editors.begin() + _currentEditor + 1);

    if (_currentEditor > 0) {
        --_currentEditor;
    }

    _inputFocus = currentEditor();
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
    if (auto editor = currentEditor()) {
        editor->cursor(fix(editor->cursor()));

        Cursor cursor = editor->cursor();
        _completeView->setCursor(cursor, *editor);
        _completeView->triggerShow(_env);
    }
}

void MainWindow::showLocator() {
    _locator->visible(true);
    _inputFocus = _locator.get();
}

void MainWindow::showCommandPalette() {
    _commandPalette->visible(true);
    _inputFocus = _commandPalette.get();
}
