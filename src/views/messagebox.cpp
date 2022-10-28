#include "views/messagebox.h"
#include "screen/draw.h"
#include "screen/iscreen.h"
#include "script/iscope.h"
#include "text/fchar.h"

MessageBox::MessageBox() {
    x(10);
    y(10);
    width(50);
    height(20);
}

void MessageBox::draw(IScreen &screen) {

    fillRect(screen, FChar{' ', 1}, x(), y(), width(), height());

    screen.draw(x() + 2, y() + 2, {"hello"});
}

bool MessageBox::keyPress(std::shared_ptr<IScope> ) {
    close();
    return true;
}

void MessageBox::updateCursor(IScreen &) const {}
