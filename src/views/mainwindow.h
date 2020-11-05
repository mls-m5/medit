#pragma once

#include "script/rootenvironment.h"
#include "views/editor.h"

struct MainWindow : public View, public IKeySink {
    Editor editor;
    RootEnvironment env;
    Editor console;
    Editor commandBuffer;
    size_t split = 10;

    IKeySink *inputFocus = &editor;
    //    IKeySink *inputFocus = &commandBuffer;

    FString splitString;

    MainWindow(size_t w, size_t h);

    ~MainWindow() override = default;

    void resize() {
        //! Todo: Handle layouts better in the future
        editor.width(width());
        if (env.showConsole()) {
            editor.height(height() - split);
        }
        else {
            editor.height(height() - 1);
        }
        editor.x(0);
        editor.y(0);

        console.width(width());
        console.height(split - 1); // 1 character for toolbar
        console.x(0);
        console.y(height() - split + 1);

        commandBuffer.x(0);
        commandBuffer.y(0);
        commandBuffer.width(width());
        commandBuffer.height(1);
    }

    //! @see IView
    void draw(IScreen &screen) override;

    //! @see IKeySink
    void updateCursor(IScreen &screen) const override {
        inputFocus->updateCursor(screen);
    }

    //! @see IKeySink
    bool keyPress(IEnvironment &env) override {
        return inputFocus->keyPress(env);
    }
};
