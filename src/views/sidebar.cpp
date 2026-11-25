#include "sidebar.h"
#include "screen/iscreen.h"
#include "syntax/palette.h"
#include "text/fstring.h"

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

    const auto title = FString{" Project", Palette::identifier};
    if (contentWidth > 0) {
        screen.draw(x(), y(), title.substr(0, contentWidth));
    }
}
