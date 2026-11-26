#pragma once

#include "keys/ikeysink.h"
#include "views/view.h"

class IScreen;

// A window region for tools, independent of the text-editor split list.
class SideBar : public View, public IKeySink {
    bool _focused = false;

public:
    explicit SideBar(IView *parent)
        : View{parent} {}

    void draw(IScreen &screen) override;

    void focused(bool value) {
        _focused = value;
    }
    bool focused() const {
        return _focused;
    }

    bool keyPress(std::shared_ptr<IEnvironment> env) override;
    void updateCursor(IScreen &screen) const override;
    bool mouseDown(int x, int y) override;
};
