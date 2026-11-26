#include "sidebar.h"
#include "keys/event.h"
#include "screen/iscreen.h"
#include "script/ienvironment.h"
#include "syntax/palette.h"
#include "text/fstring.h"
#include "views/mainwindow.h"

bool SideBar::keyPress(std::shared_ptr<IEnvironment> env) {
    if (env->key() == KeyEvent{Key::Escape}) {
        env->mainWindow().focusEditor();
        return true;
    }
    return false;
}

void SideBar::draw(IScreen &screen) {
    if (!visible() || width() == 0 || height() == 0) {
        return;
    }

    const auto contentWidth = width() - 1;
    const auto fill = FString{contentWidth, FChar{' ', Palette::standard}};
    for (size_t row = 0; row < height(); ++row) {
        if (contentWidth > 0) {
            screen.draw(x(), y() + row, fill);
        }
        screen.draw(
            x() + contentWidth, y() + row, FString{"|", Palette::lineNumbers});
    }

    const auto title = FString{
        " Project", _focused ? Palette::currentLine : Palette::identifier};
    if (contentWidth > 0) {
        screen.draw(x(), y(), title.substr(0, contentWidth));
    }
}

void SideBar::updateCursor(IScreen &screen) const {
    if (visible() && width() > 1 && height() > 0) {
        screen.cursor(x() + 1, y());
    }
}

bool SideBar::mouseDown(int mouseX, int mouseY) {
    // The separator is not part of the tool's clickable area.
    return visible() && width() > 1 && mouseX >= 0 && mouseY >= 0 &&
           static_cast<size_t>(mouseX) >= x() &&
           static_cast<size_t>(mouseX) - x() < width() - 1 &&
           static_cast<size_t>(mouseY) >= y() &&
           static_cast<size_t>(mouseY) - y() < height();
}
