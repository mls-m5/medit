#pragma once

#include "completion/autocomplete.h"
#include "completion/icompletionsource.h"
#include "core/plugins.h"
#include "meditfwd.h"
#include "text/cursor.h"
#include "views/listview.h"
#include "views/view.h"

class CompleteView : public View, IKeySink {
public:
    struct CompletionResult {
        std::string value;
        std::string postfix;
    };

    CompleteView(IView *parent, Plugins::ListT<ICompletionSource>);

    //! @param cursorPosition is the cursors position _on the screen_
    void triggerShow(std::shared_ptr<IEnvironment>);

    void updateCompletion(std::string str);

    //! @see IView
    void draw(IScreen &screen) override {
        _list.draw(screen);
    }

    //! @see IKeySink
    bool keyPress(std::shared_ptr<IEnvironment>) override;

    //! @see IKeySink
    void updateCursor(IScreen &screen) const override {
        _list.updateCursor(screen);
    }

    void callback(std::function<void(CompletionResult)> f);

    void setCursor(Cursor cursor, const Editor &bufferView);

    bool mouseDown(int x, int ty) override {
        return false;
    }

private:
    void currentText(std::string str);

    ListView _list;
    AutoComplete _autoComplete;
    std::string _currentText;
};
