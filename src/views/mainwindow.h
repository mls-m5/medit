#pragma once

#include "files/projectfiles.h"
#include "locator.h"
#include "script/rootenvironment.h"
#include "views/completeview.h"
#include "views/editor.h"
#include "views/listview.h"

struct MainWindow : public View, public IKeySink {
    Editor _editor;
    RootEnvironment _env;
    Editor _console;
    Locator _locator;
    CompleteView _completeView;
    ProjectFiles _project;
    size_t _split = 10;

    //    IKeySink *_inputFocus = &_testList;
    IKeySink *_inputFocus = &_editor;
    //    IKeySink *_inputFocus = &_commandBuffer;

    FString splitString;

    MainWindow(size_t w, size_t h);

    ~MainWindow() override = default;

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

private:
    void addCommands();
};
