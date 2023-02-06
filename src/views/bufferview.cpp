#include "views/bufferview.h"
#include "screen/iscreen.h"
#include "syntax/ipalette.h"
#include "text/buffer.h"
#include "text/cursor.h"
#include "text/cursorrange.h"

namespace {

size_t getLineNumWidth(size_t numLines) {
    size_t width = 0;
    while (numLines /= 10) {
        ++width;
    }
    return std::min<size_t>(width + 1, 2);
}

} // namespace

BufferView::~BufferView() = default;

BufferView::BufferView(IView *parent, std::shared_ptr<Buffer> buffer)
    : ScrollView{parent}
    , _buffer(std::move(buffer)) {}

void BufferView::draw(IScreen &screen) {
    if (!visible()) {
        return;
    }

    _numberWidth = getLineNumWidth(_buffer->lines().size()) + 2;
    auto fillStr =
        FString{std::string(_numberWidth, ' '), IPalette::lineNumbers};

    auto fillStrError =
        FString{std::string(_numberWidth, ' '), IPalette::error};
    for (size_t ty = 0; ty < height(); ++ty) {
        auto l = ty + yScroll();
        if (l < buffer().lines().size()) {
            auto hasLineDiagnostics =
                buffer().diagnostics().findLineDiagnostic(l - 1);

            auto &line = _buffer->lines().at(l);

            screen.draw(x() + _numberWidth, y() + ty, line);

            if (hasLineDiagnostics) {
                /// TODO: Fix this, just testing
                screen.draw(
                    x() + _numberWidth + line.size() + 2,
                    y() + ty,
                    FString{hasLineDiagnostics->message, IPalette::comment});
            }

            if (_showLines) {
                const auto lineNum = l + 1;
                auto lineFormat = IPalette::lineNumbers;

                auto offset = 1;
                screen.draw(
                    x(), y() + ty, hasLineDiagnostics ? fillStrError : fillStr);

                screen.draw(
                    x() + offset,
                    y() + ty,
                    {std::to_string(lineNum),
                     hasLineDiagnostics ? IPalette::error : lineFormat});

                ++l;
            }
        }
    }

    //    for (auto &item : buffer().diagnostics().list()) {
    //        drawSpecial(screen,
    //                    CursorRange{buffer(), item.range.begin,
    //                    item.range.end}, IPalette::selection);

    //        break;
    //    }
}

void BufferView::drawSpecial(IScreen &screen,
                             const CursorRange &range,
                             FormatType f) const {
    if (!visible()) {
        return;
    }

    for (size_t ty = 0; ty < height(); ++ty) {
        auto l = ty + yScroll();
        bool isAffected = false;
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
                isAffected = true;
            }
            else if (l > range.begin().y() && l < range.end().y()) {
                // Selection on a full line
                line.format(f);
                isAffected = true;
            }
            else if (l == range.begin().y()) {
                // Selection on first line in multi line selection
                for (auto i = range.begin().x(); i < line.size(); ++i) {
                    line.at(i).f = f;
                    isAffected = true;
                }
            }
            else if (l == range.end().y()) {
                // Selection on last line in multi line selection
                for (size_t i = 0; i < range.end().x() && i < line.size();
                     ++i) {
                    line.at(i).f = f;
                    isAffected = true;
                }
            }
            if (!line.empty() && isAffected) {
                screen.draw(x() + _numberWidth, y() + ty, line);
            }
        }
    }
}

Buffer &BufferView::buffer() {
    return *_buffer;
}

const Buffer &BufferView::buffer() const {
    return *_buffer;
}

void BufferView::buffer(std::shared_ptr<Buffer> buffer) {
    _buffer = std::move(buffer);
}

size_t BufferView::numberWidth() const {
    return _numberWidth;
}

void BufferView::showLines(bool value) {
    _showLines = value;
    _numberWidth = value * 3;
}

Position BufferView::cursorPosition(Cursor cursor) const {
    return {x() + cursor.x() + _numberWidth, y() + cursor.y() - yScroll()};
}
