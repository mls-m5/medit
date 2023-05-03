#include "screen/draw.h"
#include "screen/iscreen.h"
#include "views/iview.h"

void fillRect(IScreen &screen,
              const FChar &fill,
              size_t x,
              size_t y,
              size_t width,
              size_t height) {
    auto str = FString{width, fill};

    for (size_t i = 0; i < height; ++i) {
        screen.draw(x, y + i, str);
    }
}

void fillRect(IScreen &screen, const FChar &c, const IView &view) {
    fillRect(screen, c, view.x(), view.y(), view.width(), view.height());
}

void draw(IScreen &screen, size_t x, size_t y, FStringView str) {
    screen.draw(x, y, str);
}
