#include "views/bufferview.h"
#include "screen/iscreen.h"
#include "syntax/ipalette.h"
#include "text/buffer.h"
#include "text/cursor.h"
#include "text/cursorrange.h"

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

void BufferView::drawSpecial(IScreen &screen,
                             CursorRange &range,
                             FormatType f) {
    if (!visible()) {
        return;
    }

    for (size_t ty = 0; ty < height(); ++ty) {
        auto l = ty + yScroll();
        if (l < buffer().lines().size()) {
            auto line = _buffer->lines().at(l);
            if (range.begin().y() == range.end().y() &&
                l == range.begin().y()) {
                // Selection on a single line
                for (auto i = range.begin().x();
                     i < line.size() && i < range.endPosition().x();
                     ++i) {
                    line.at(i).f = f;
                }
            }
            else if (l > range.begin().y() && l < range.end().y()) {
                // Selection on a full line
                line.format(f);
            }
            else if (l == range.begin().y()) {
                // Selection on first line in multi line selection
                for (auto i = range.begin().x(); i < line.size(); ++i) {
                    line.at(i).f = f;
                }
            }
            else if (l == range.end().y()) {
                // Selection on last line in multi line selection
                for (size_t i = 0; i < range.end().x() && i < line.size();
                     ++i) {
                    line.at(i).f = f;
                }
            }
            screen.draw(x() + _numberWidth, y() + ty, line);
        }
    }
}

void BufferView::buffer(std::unique_ptr<Buffer> buffer) {
    _buffer = std::move(buffer);
}

Position BufferView::cursorPosition(Cursor cursor) const {
    return {x() + cursor.x() + _numberWidth, y() + cursor.y() - yScroll()};
}
