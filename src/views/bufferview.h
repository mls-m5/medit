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

    auto x() const {
        return _x;
    }

    auto y() const {
        return _y;
    }

private:
    std::unique_ptr<Buffer> _buffer;

    size_t _x = 0;
    size_t _y = 0;
};
