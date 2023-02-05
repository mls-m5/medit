#pragma once

#include "window.h"

class MessageBox : public Window {
public:
    MessageBox();

    //! @see IView interface
    void draw(IScreen &) override;

    //! @see IKeySink interface
    bool keyPress(std::shared_ptr<IScope> env) override;

    //! @see IKeySink interface
    void updateCursor(IScreen &screen) const override;

    bool mouseDown(int x, int ty) override {
        return false;
    }
};
