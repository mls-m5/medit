#pragma once

#include "files/project.h"
#include "locator.h"
#include "script/rootenvironment.h"
#include "syntax/IFormat.h"
#include "views/completeview.h"
#include "views/editor.h"
#include "views/listview.h"
#include "views/window.h"

struct MainWindow : public Window {
    Editor _editor;
    RootEnvironment _env;
    Editor _console;
    Locator _locator;
    CompleteView _completeView;
    Project _project;
    size_t _split = 10;
    FString splitString;
    IKeySink *_inputFocus = &_editor;
    std::unique_ptr<IWindow> _activePopup;

    //! Move these
    std::vector<std::unique_ptr<IHighlight>> _highlighting;
    std::vector<std::unique_ptr<IFormat>> _formatting;

    MainWindow(size_t w, size_t h);

    ~MainWindow() override;

    //! Set width and height to refresh but keep old size
    void resize(size_t width = 0, size_t height = 0);

    //! @see IView
    void draw(IScreen &screen) override;

    //! @see IKeySink
    void updateCursor(IScreen &screen) const override;

    //! @see IKeySink
    bool keyPress(IEnvironment &env) override;

    void updateLocatorBuffer();

    void open(filesystem::path path);

    void updateHighlighting();

    void showPopup(std::unique_ptr<IWindow> popup);

    void resetFocus();

private:
    void addCommands();
};
