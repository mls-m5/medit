#pragma once

#include "script/environment.h"
#include "views/editor.h"

class Locator : public Editor {
public:
    Environment _env;

    Locator(IEnvironment &env);

    // IKeySink interface
public:
    bool keyPress(IEnvironment &) {
        return Editor::keyPress(_env);
    }
};
