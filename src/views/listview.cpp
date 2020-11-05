
#include "views/listview.h"
#include "screen/iscreen.h"
#include "script/ienvironment.h"

struct ListView::ListItem {
    FString text;
    std::any content;
};

ListView::ListView() = default;

ListView::~ListView() = default;

void ListView::draw(IScreen &screen) {
    size_t ty = 0;

    for (const auto &l : _lines) {
        screen.draw(x(), y() + ty, l.text);
        ++ty;
    }
}

bool ListView::keyPress(IEnvironment &env) {
    switch (env.key().key) {
    case Key::Up:
        // do up
        return true;
    case Key::Down:
        // do Down
        return true;
    default:
        return false;
    }
}

void ListView::updateCursor(IScreen &screen) const {
    screen.cursor(x(), y() - yScroll());
}
