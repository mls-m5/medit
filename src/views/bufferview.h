#pragma once

#include "meditfwd.h"
#include "text/buffer.h"
#include "views/view.h"
#include <memory>

class BufferView : public View {
public:
    BufferView(std::unique_ptr<Buffer> buffer) : _buffer(std::move(buffer)) {}
    BufferView(const BufferView &) = delete;
    BufferView &operator=(const BufferView &) = delete;
    BufferView(BufferView &&) = default;
    BufferView &operator=(BufferView &&) = default;
    ~BufferView() override;

    void draw(IScreen &) override;

    Buffer &buffer() {
        return *_buffer;
    }

    const Buffer &buffer() const {
        return *_buffer;
    }

    auto numberWidth() const {
        return _numberWidth;
    }

    void scrollPosition(size_t value) {
        _scrollPosition = value;
    }

    auto scrollPosition() const {
        return _scrollPosition;
    }

    void showLines(bool value) {
        _showLines = value;
        _numberWidth = value * 3;
    }

private:
    std::unique_ptr<Buffer> _buffer;

    bool _showLines = true;
    size_t _numberWidth = 3;
    size_t _scrollPosition = 0;
};
