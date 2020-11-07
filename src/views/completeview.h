#pragma once

#include "text/autocomplete.h"
#include "text/cursor.h"
#include "views/bufferview.h"
#include "views/listview.h"
#include "views/view.h"

class CompleteView : public View, IKeySink {
public:
    struct CompletionResult {
        std::string value;
        std::string postfix;
    };

    CompleteView() {
        _list.width(20);
        _list.height(20);
    }

    void triggerShow(Position cursorPosition) {
        //        _cursorPosition = bufferView.cursorPosition(cursor);
        _cursorPosition = cursorPosition;
        _list.visible(true);
        visible(true);

        //! Make this position next to cursor
        _list.x(cursorPosition.x());
        _list.y(cursorPosition.y() + 1);
        _list.current(0);

        updateCompletion("");
    }

    void updateCompletion(std::string str) {
        auto values = _autoComplete.getMatching(str);
        auto selected = _list.current();
        _list.clear();
        for (auto &item : values) {
            _list.addLine(item.name, item.name);
        }
        _list.current(selected);
    }

    //! @see IView
    void draw(IScreen &screen) override {
        _list.draw(screen);
    }

    //! @see IKeySink
    bool keyPress(IEnvironment &env) override {
        return _list.keyPress(env);
    }

    //! @see IKeySink
    void updateCursor(IScreen &screen) const override {
        _list.updateCursor(screen);
    }

    void callback(std::function<void(CompletionResult)> f) {
        _list.callback([f, this](auto &&, auto &&value) {
            auto str = std::any_cast<std::string>(value);
            f({str, ""});
            visible(false);
            _list.visible(false);
        });
    }

private:
    ListView _list;
    AutoComplete _autoComplete;
    Position _cursorPosition;
};
