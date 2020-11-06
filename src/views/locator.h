#pragma once

#include "script/environment.h"
#include "views/editor.h"
#include "views/listview.h"

class Locator : public Editor {
public:
    Environment _env;
    ListView _list;

    Locator(IEnvironment &env);
    ~Locator() override;

    // @see IKeySink
    bool keyPress(IEnvironment &) override;

    // @see IView
    void draw(IScreen &) override;

    void callback(std::function<void(std::string)> f) {
        _list.callback([f](auto &&str, auto &&) { f(str); });
    }

    void visible(bool value);
};
