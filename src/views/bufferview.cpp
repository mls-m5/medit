
#include "views/bufferview.h"
#include "screen/iscreen.h"
#include "text/buffer.h"

BufferView::~BufferView() = default;

void BufferView::draw(IScreen &window) {
    size_t l = 0;

    for (auto &line : _buffer->lines()) {
        window.draw(_x + _numberWidth, _y + l, line);
        window.draw(_x + 1, _y + l, {std::to_string(l + 1), 3});
        ++l;
    }
}
