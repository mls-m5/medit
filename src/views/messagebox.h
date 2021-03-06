#pragma once

#include "window.h"

class MessageBox : public Window {
public:
    MessageBox();

    //! @see IView interface
    void draw(IScreen &);

    //! @see IKeySink interface
    bool keyPress(std::shared_ptr<IEnvironment> env);

    //! @see IKeySink interface
    void updateCursor(IScreen &screen) const;
};
