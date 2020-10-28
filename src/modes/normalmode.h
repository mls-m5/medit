#pragma once

#include "keys/keyevent.h"
#include "modes/imode.h"
#include "script/environment.h"

class NormalMode : public IMode {
    IEnvironment &_env;

public:
    NormalMode(IEnvironment &env) : _env(env) {}

    void keyPress(const KeyEvent &c, Editor &) override;

    std::string_view name() override;
};
