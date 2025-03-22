#pragma once

#include "text/fstring.h"
#include "views/editor.h"
#include "views/window.h"
#include <functional>

class InputBox : public Window {
public:
    InputBox(const InputBox &) = delete;
    InputBox(InputBox &&) = delete;
    InputBox &operator=(const InputBox &) = delete;
    InputBox &operator=(InputBox &&) = delete;

    InputBox(IWindow *parent, FString title, std::string defaultValue = {});
    ~InputBox();

    //! @see IKeySink interface
    bool keyPress(std::shared_ptr<IEnvironment>) override;
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

    Editor *currentEditor() override {
        return &_entry;
    }

private:
    Editor _entry;
    std::function<void(std::string)> _callback;
    FString _title;
};
