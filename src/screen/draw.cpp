#include "screen/draw.h"
#include "screen/iscreen.h"

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
