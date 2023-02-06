#pragma once

#include "text/fstring.h"
#include "views/editor.h"
#include "views/window.h"
#include <functional>

class InputBox : public Window {
public:
    InputBox(IWindow *parent, FString title, std::string defaultValue = {});
    ~InputBox();

    //! @see IKeySink interface
    bool keyPress(std::shared_ptr<IScope>) override;
    void updateCursor(IScreen &screen) const override;

    //! @see IView
    void draw(IScreen &) override;

    void callback(std::function<void(std::string)> callback) {
        _callback = callback;
    }

    // TODO: Implement
    bool mouseDown(int x, int ty) override {
        return false;
    }

private:
    Editor _entry;
    std::function<void(std::string)> _callback;
    FString _title;
};
