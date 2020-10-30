
#include "views/bufferview.h"
#include "screen/iscreen.h"
#include "text/buffer.h"

BufferView::~BufferView() = default;

void BufferView::draw(IScreen &window) {
    size_t l = 0;

    for (auto &line : _buffer->lines()) {
        window.draw(_x + _numberWidth, _y + l, line);
        const auto lineNum = l + 1;
        size_t fill = 0;
        if (lineNum < 10) {
            fill += 1;
        }
        window.draw(
            _x,
            _y + l,
            {std::string(fill, ' ') + std::to_string(lineNum) + " ", 3});
        ++l;
    }

    auto fillStr = FString{std::string(width(), ' '), 5};

    for (size_t y = _buffer->lines().size(); y < height(); ++y) {
        window.draw(0, y, fillStr);
    }
}
