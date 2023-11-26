#include "views/bufferview.h"
#include "screen/iscreen.h"
#include "syntax/palette.h"
#include "text/buffer.h"
#include "text/cursor.h"
#include "text/cursorrange.h"
#include "text/fchar.h"
#include "text/fstring.h"
#include "views/iwindow.h"
#include <algorithm>
#include <cstddef>
#include <stdexcept>

namespace {

size_t getLineNumWidth(size_t numLines) {
    size_t width = 0;
    while (numLines /= 10) {
        ++width;
    }
    return std::max<size_t>(width + 1, 2);
}

} // namespace

BufferView::~BufferView() = default;

BufferView::BufferView(IView *parent, std::shared_ptr<Buffer> buffer)
    : ScrollView{parent}
    , _buffer(std::move(buffer)) {
    subscribeToBuffer();
}

void BufferView::draw(IScreen &screen) {
    if (!visible()) {
        return;
    }

    _numberWidth = getLineNumWidth(_buffer->lines().size()) + 2;
    auto fillStr =
        FString{std::string(_numberWidth, ' '), Palette::lineNumbers};

    const auto maxWidth = 80;
    auto overSize = static_cast<std::ptrdiff_t>(width() + xScroll()) -
                    maxWidth - static_cast<std::ptrdiff_t>(_numberWidth);
    auto overWidthFill = FString{};
    if (overSize > 0) {
        overWidthFill = FString{static_cast<std::size_t>(overSize),
                                FChar{' ', Palette::outside}};
    }

    auto bottomFill = FString{width(), FChar{' ', Palette::outside}};

    auto fillStrError = FString{std::string(_numberWidth, ' '), Palette::error};
    for (size_t ty = 0; ty < height(); ++ty) {
        auto l = ty + yScroll();
        if (l < buffer().lines().size()) {
            auto hasLineDiagnostics =
                buffer().diagnostics().findLineDiagnostic(l);

            auto &line = _buffer->lines().at(l);

            if (overSize > 0) {
                screen.draw(x() + _numberWidth + maxWidth - xScroll(),
                            y() + ty,
                            overWidthFill);
            }

            screen.draw(x() + _numberWidth, y() + ty, line.substr(xScroll()));

            if (hasLineDiagnostics) {
                /// TODO: Fix this, just testing
                screen.draw(
                    x() + _numberWidth + line.size() + 2,
                    y() + ty,
                    FString{hasLineDiagnostics->message, Palette::comment});
            }

            if (_showLines) {
                const auto lineNum = l + 1;
                auto lineFormat = Palette::lineNumbers;

                auto offset = 1;
                screen.draw(
                    x(), y() + ty, hasLineDiagnostics ? fillStrError : fillStr);

                screen.draw(
                    x() + offset,
                    y() + ty,
                    FString{std::to_string(lineNum),
                            hasLineDiagnostics ? Palette::error : lineFormat});

                ++l;
            }
        }
        else {
            screen.draw(x(), y() + ty, bottomFill);
        }
    }
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
        bool shouldDrawNewline = false;
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
                shouldDrawNewline = true;
            }
            else if (l == range.begin().y()) {
                // Selection on first line in multi line selection
                for (auto i = range.begin().x(); i < line.size(); ++i) {
                    line.at(i).f = f;
                    isAffected = true;
                }
                shouldDrawNewline = true;
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
            if (shouldDrawNewline) {
                /// Draw the newline after
                screen.draw(x() + _numberWidth + line.size(),
                            y() + ty,
                            FString{" ", f});
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
    if (!buffer) {
        throw std::runtime_error{"trying to set buffer view buffer to nullptr"};
    }
    unsubscribe();
    _buffer = std::move(buffer);
    subscribeToBuffer();
    contentHeight(_buffer->lines().size());

    //    if (auto w = window()) { // Why does this not work?
    //        w->triggerRedraw();
    //    }
}

size_t BufferView::numberWidth() const {
    return _numberWidth;
}

void BufferView::showLines(bool value) {
    _showLines = value;
    _numberWidth = value * 3;
}

Position BufferView::cursorPosition(Position cursor) const {
    return {x() + cursor.x() + _numberWidth, y() + cursor.y() - yScroll()};
}

void BufferView::subscribeToBuffer() {
    _buffer->subscribe([this]() { bufferChangedEvent(); }, this);
}

void BufferView::unsubscribe() {
    _buffer->unsubscribe(this);
}

void BufferView::bufferChangedEvent() {
    contentHeight(_buffer->lines().size());

    if (auto w = window()) {
        w->triggerRedraw();
    }
}
