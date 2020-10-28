#pragma once

#include "keys/keyevent.h"
#include "modes/imode.h"

class InsertMode : public IMode {
public:
    void keyPress(const KeyEvent c, Editor &) override;

    std::string_view name() override;
};
