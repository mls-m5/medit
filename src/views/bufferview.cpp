#include "views/bufferview.h"
#include "core/profiler.h"
#include "files/extensions.h"
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
#include <limits>
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
    PROFILE_FUNCTION();

    auto &lines = _virtualLines;

    _numberWidth = getLineNumWidth(lines.size()) + 2;
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
        if (l < lines.size()) {
            auto hasLineDiagnostics =
                buffer().diagnostics().findLineDiagnostic(l);

            auto &vline = lines.at(l);
            auto &line = vline.text;

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

            if (_showLines && vline.start == 0) {
                const auto lineNum = vline.lineNum + 1;
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
    PROFILE_FUNCTION();
    if (!visible()) {
        return;
    }

    auto &lines = _virtualLines;

    for (size_t ty = 0; ty < height(); ++ty) {
        auto l = ty + yScroll();
        bool isAffected = false;
        bool shouldDrawNewline = false;
        if (l < lines.size()) {
            auto line = FString{lines.at(l).text};
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
    bufferChangedEvent();

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

Position BufferView::screenPositionFromCursor(Position cursor) const {
    return {x() + cursor.x() + _numberWidth, y() + cursor.y() - yScroll()};
}

Cursor BufferView::cursorFromScreenPosition(Position pos) const {
    if (_virtualLines.empty()) {
        return {*_buffer};
    }

    auto local = localFromScreenPosition(pos);

    if (local.y() + 1 > _virtualLines.size()) {
        local.y(_virtualLines.size() - 1);
    }

    auto &vline = _virtualLines.at(local.y());

    if (local.x() > vline.text.size()) {
        local.x(vline.text.size());
    }

    return {*_buffer, vline.start + local.x(), vline.lineNum};
}

Position BufferView::localFromScreenPosition(Position pos) const {
    pos.x(pos.x() + xScroll());
    pos.y(pos.y() + yScroll());
    auto offsetX = _numberWidth + x();
    auto offsetY = y();
    offsetX = std::min(pos.x(), offsetX);
    offsetY = std::min(pos.y(), offsetY);
    return {pos.x() - offsetX, pos.y() - offsetY};
}

Position BufferView::cursorToScreen(Position pos) const {
    pos = cursorToLocal(pos);
    return {x() + _numberWidth + pos.x() - xScroll(),
            y() + pos.y() - yScroll()};
}

Position BufferView::cursorToLocal(Position pos) const {
    if (_virtualLineLookup.empty()) {
        return {0, 0};
    }

    auto y = std::min(pos.y(), _virtualLineLookup.size() - 1);

    auto virtualLineIndex = _virtualLineLookup.at(y);

    auto nextLine = _virtualLines.size();
    if (y + 1 < _virtualLineLookup.size()) {
        nextLine = _virtualLineLookup.at(y + 1);
    }

    for (size_t i = virtualLineIndex; i < nextLine; ++i) {
        auto &vline = _virtualLines.at(i);
        if (pos.x() >= vline.start) {
            virtualLineIndex = i;
        }
        else {
            break;
        }
    }

    auto &vline = _virtualLines.at(virtualLineIndex);

    return {pos.x() - vline.start, virtualLineIndex};
}

bool BufferView::shouldWrap() const {
    auto path = _buffer->path();
    return isMarkdown(path) || isTextFile(path);
}

void BufferView::subscribeToBuffer() {
    _buffer->subscribe([this]() { bufferChangedEvent(); }, this);
}

void BufferView::unsubscribe() {
    _buffer->unsubscribe(this);
}

void BufferView::bufferChangedEvent() {
    rewrapLines();

    if (auto w = window()) {
        w->triggerRedraw();
    }
}

void BufferView::rewrapLines() {
    PROFILE_FUNCTION();
    auto maxLineWidth = width() - _numberWidth;
    maxLineWidth = std::min(_maxWrapLength, maxLineWidth);
    if (!shouldWrap()) {
        maxLineWidth = std::numeric_limits<size_t>::max();
    }

    _virtualLines.clear();
    _virtualLineLookup.clear();

    auto splitLine = [&](FStringView str, size_t line) {
        // Naive implementation
        for (size_t i = 0; i < str.size(); i += maxLineWidth) {
            _virtualLines.push_back({
                .text = str.substr(i, maxLineWidth),
                .start = i,
                .lineNum = line,
            });
        }
    };

    auto &lines = _buffer->lines();
    for (size_t i = 0; i < lines.size(); ++i) {
        auto &line = lines.at(i);
        _virtualLineLookup.push_back(_virtualLines.size());
        if (line.size() > maxLineWidth) {
            splitLine(line, i);
        }
        else {
            _virtualLines.push_back({
                .text = line,
                .start = 0,
                .lineNum = i,
            });
        }
    }

    contentHeight(_virtualLines.size());
}

void BufferView::onResize(size_t oldWidth, size_t oldHeight) {
    if (oldWidth != width()) {
        rewrapLines();
    }
}
