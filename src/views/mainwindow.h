#pragma once

#include "files/project.h"
#include "interactionhandling.h"
#include "meditfwd.h"
#include "script/localenvironment.h"
#include "views/completeview.h"
#include "views/editor.h"
#include "views/locator.h"
#include "views/window.h"
#include <memory>

struct MainWindow : public Window {
    IScreen &_screen;
    std::vector<std::shared_ptr<Editor>>
        _editors; // Shared ptr to allow for weak ptr
    InteractionHandling _interactions;
    std::shared_ptr<LocalEnvironment> _env;
    Editor _console;
    Locator _locator;
    CompleteView _completeView;
    Project _project;
    size_t _split = 10;
    FString _splitString;
    IKeySink *_inputFocus = nullptr;
    std::unique_ptr<IWindow> _activePopup;
    std::vector<std::weak_ptr<Buffer>> _buffersToUpdate;
    size_t _currentEditor = 0;
    bool _shouldRedraw = true;

    MainWindow(IScreen &screen, ThreadContext &context);

    ~MainWindow() override;

    //! Set width and height to refresh but keep old size
    void resize(size_t width = 0, size_t height = 0);

    //! @see IView
    void draw(IScreen &screen) override;

    //! @see IKeySink
    void updateCursor(IScreen &screen) const override;

    //! @see IKeySink
    bool keyPress(std::shared_ptr<IEnvironment>) override;

    void updateLocatorBuffer();

    void open(filesystem::path path,
              std::optional<int> x = {},
              std::optional<int> y = {});

    void showPopup(std::unique_ptr<IWindow> popup);

    Editor *currentEditor() override;

    void resetFocus();

    void switchEditor();

    void paste(std::string text);

    bool mouseDown(int x, int y) override;
    void copy(bool shouldCut);

    // Window management ----------

    //! Queue up that the window should be redrawn
    //! Sets _shouldRedraw = true and force guithread to work
    void triggerRedraw() override;

    /// Redraw screen if _shouldRefraw is true
    void refreshScreen();

    IEnvironment &env() override {
        return *_env;
    }

    IWindow *window() override {
        return this;
    }

    InteractionHandling &interactions() {
        return _interactions;
    }

    void updateTitle();

    // Commands --------------------
    void escape();
    void showConsole();

    /// Show open dialogue
    void showOpen();

    void gotoDefinition();

    /// User request to format file
    void format();

    /// User want to auto complete
    void autoComplete();

    void showLocator();
};
