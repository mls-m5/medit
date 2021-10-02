#pragma once

#include "files/project.h"
#include "meditfwd.h"
#include "script/rootenvironment.h"
#include "syntax/iformat.h"
#include "views/completeview.h"
#include "views/editor.h"
#include "views/listview.h"
#include "views/locator.h"
#include "views/window.h"

struct MainWindow : public Window {
    std::vector<Editor> _editors;
    std::shared_ptr<RootEnvironment> _env;
    Editor _console;
    Locator _locator;
    CompleteView _completeView;
    Project _project;
    size_t _split = 10;
    FString _splitString;
    IKeySink *_inputFocus = &_editors.front();
    std::unique_ptr<IWindow> _activePopup;
    size_t _updateTimeHandle = 0;
    size_t _currentEditor = 0;

    //! Move these somewhere else
    std::vector<std::unique_ptr<IHighlight>> _highlighting;
    std::vector<std::unique_ptr<IFormat>> _formatting;
    std::vector<std::unique_ptr<IAnnotation>> _annotation;
    std::vector<std::unique_ptr<INavigation>> _navigation;

    MainWindow(IScreen &screen, Context &context);

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

    void open(filesystem::path path);

    //! Initialize palette colors
    void updatePalette(IScreen &screen);

    //! Update editor with new highlighting
    void updateHighlighting(Editor &editor);

    void showPopup(std::unique_ptr<IWindow> popup);

    Editor &currentEditor();

    void resetFocus();

    void switchEditor();

private:
    void addCommands(IScreen &screen);
};
