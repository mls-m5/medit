
#include "views/bufferview.h"
#include "buffer.h"
#include "screen/iscreen.h"

BufferView::~BufferView() = default;

void BufferView::draw(IScreen &window) {
    size_t l = 0;

    for (auto &line : _buffer->lines()) {
        window.draw(_x, _y + l, line);
        ++l;
    }
}
