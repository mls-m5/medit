#pragma once

#include "keys/keyevent.h"
#include "modes/imode.h"

#include <memory>

std::unique_ptr<IMode> createInsertMode();

class InsertMode : public IMode {
public:
    InsertMode() {}

    void keyPress(IEnvironment &) override;

    std::string_view name() override;
};
