#pragma once

#include "meditfwd.h"
#include "window.h"

class MessageBox : public Window {
public:
    MessageBox(IWindow &parent);

    //! @see IView interface
    void draw(IScreen &) override;

    //! @see IKeySink interface
    bool keyPress(std::shared_ptr<IEnvironment> env) override;

    //! @see IKeySink interface
    void updateCursor(IScreen &screen) const override;

    bool mouseDown(int x, int ty) override {
        return false;
    }

    void triggerRedraw() override {}
};
