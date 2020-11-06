
#include "locator.h"
#include "modes/parentmode.h"

Locator::Locator(IEnvironment &env) : _env(&env) {
    _env.editor(this);
    _list.addLine("hello");
    _list.addLine("there");
    _list.x(0);
    _list.y(1);
    _list.width(20);
    _list.height(20);
}

bool Locator::keyPress(IEnvironment &) {
    if (_list.keyPress(_env)) {
        return true;
    }
    else {
        return Editor::keyPress(_env);
    }
}

Locator::~Locator() = default;

void Locator::draw(IScreen &screen) {
    if (Editor::visible()) {
        Editor::draw(screen);
        _list.draw(screen);
    }
}

void Locator::visible(bool value) {
    _list.visible(value);
    Editor::visible(value);
}
