#include "views/bufferview.h"
#include "screen/iscreen.h"
#include "text/buffer.h"

BufferView::~BufferView() = default;

void BufferView::draw(IScreen &window) {
    if (!visible()) {
        return;
    }
    auto fillStr = FString{std::string(width(), ' '), 5};
    for (size_t ty = 0; ty < height(); ++ty) {
        auto l = ty + yScroll();
        if (l < buffer().lines().size()) {
            auto &line = _buffer->lines().at(l);

            window.draw(x() + _numberWidth, y() + ty, line);

            if (_showLines) {
                const auto lineNum = l + 1;
                size_t fill = 0;
                if (lineNum < 10) {
                    fill += 1;
                }
                window.draw(
                    x(),
                    y() + ty,
                    {std::string(fill, ' ') + std::to_string(lineNum) + " ",
                     3});
                ++l;
            }
        }
        else {
            window.draw(0, y() + ty, fillStr);
        }
    }
}
