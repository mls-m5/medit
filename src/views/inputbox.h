#pragma once

#include "text/fstring.h"
#include "views/editor.h"
#include "views/window.h"
#include <functional>

class InputBox : public Window {
public:
    InputBox(FString title, std::string defaultValue = {});
    ~InputBox();

    //! @see IKeySink interface
    bool keyPress(IEnvironment &env);
    void updateCursor(IScreen &screen) const;

    //! @see IView
    void draw(IScreen &);

    void callback(std::function<void(std::string)> callback) {
        _callback = callback;
    }

private:
    Editor _entry;
    std::function<void(std::string)> _callback;
    FString _title;
};
