#include "views/bufferview.h"
#include "screen/iscreen.h"
#include "syntax/ipalette.h"
#include "text/buffer.h"
#include "text/cursor.h"

BufferView::~BufferView() = default;

BufferView::BufferView(std::unique_ptr<Buffer> buffer)
    : _buffer(std::move(buffer)) {}

void BufferView::draw(IScreen &screen) {
    if (!visible()) {
        return;
    }
    auto fillStr = FString{std::string(width(), ' '), 5};
    for (size_t ty = 0; ty < height(); ++ty) {
        auto l = ty + yScroll();
        if (l < buffer().lines().size()) {
            auto &line = _buffer->lines().at(l);

            screen.draw(x() + _numberWidth, y() + ty, line);

            if (_showLines) {
                const auto lineNum = l + 1;
                auto lineFormat = IPalette::lineNumbers;
                size_t fill = 0;
                if (lineNum < 10) {
                    fill += 1;
                }
                screen.draw(
                    x(),
                    y() + ty,
                    {std::string(fill, ' ') + std::to_string(lineNum) + " ",
                     lineFormat});
                ++l;
            }
        }
        else {
            screen.draw(0, y() + ty, fillStr);
        }
    }
}

Position BufferView::cursorPosition(Cursor cursor) const {
    return {x() + cursor.x() + _numberWidth, y() + cursor.y() - yScroll()};
}
