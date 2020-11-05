#pragma once

#include "script/rootenvironment.h"
#include "views/editor.h"
#include "views/listview.h"

struct MainWindow : public View, public IKeySink {
    Editor _editor;
    RootEnvironment _env;
    Editor _console;
    Editor _commandBuffer;
    ListView _testList;
    size_t _split = 10;

    IKeySink *_inputFocus = &_testList;
    //    IKeySink *inputFocus = &_editor;
    //    IKeySink *inputFocus = &commandBuffer;

    FString splitString;

    MainWindow(size_t w, size_t h);

    ~MainWindow() override = default;

    void resize();

    //! @see IView
    void draw(IScreen &screen) override;

    //! @see IKeySink
    void updateCursor(IScreen &screen) const override;

    //! @see IKeySink
    bool keyPress(IEnvironment &env) override;
};
