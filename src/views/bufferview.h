#pragma once

#include "meditfwd.h"
#include "text/buffer.h"
#include "views/iview.h"
#include <memory>

class BufferView : public virtual IView {
public:
    BufferView(std::unique_ptr<Buffer> buffer) : _buffer(std::move(buffer)) {}
    BufferView(const BufferView &) = delete;
    BufferView &operator=(const BufferView &) = delete;
    BufferView(BufferView &&) = default;
    BufferView &operator=(BufferView &&) = default;
    ~BufferView();

    void draw(IScreen &) override;

    Buffer &buffer() {
        return *_buffer;
    }

    const Buffer &buffer() const {
        return *_buffer;
    }

    size_t x() const override {
        return _x;
    }

    size_t y() const override {
        return _y;
    }

    size_t width() const override {
        return _width;
    }

    size_t height() const override {
        return _height;
    }

    void width(size_t value) {
        _width = value;
    }

    void height(size_t value) {
        _height = value;
    }

    auto numberWidth() const {
        return _numberWidth;
    }

private:
    std::unique_ptr<Buffer> _buffer;

    size_t _x = 0;
    size_t _y = 0;
    size_t _width = 0;
    size_t _height = 0;
    size_t _numberWidth = 3;
};
