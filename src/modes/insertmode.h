#pragma once

#include "keyevent.h"
#include "modes/imode.h"

class InsertMode : public IMode {
    int keyPress(KeyEvent c) override;
};
