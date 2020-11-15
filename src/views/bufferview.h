#pragma once

#include "meditfwd.h"
#include "views/scrollview.h"
#include <memory>

class BufferView : public ScrollView {
public:
    BufferView(std::unique_ptr<Buffer> buffer);
    ~BufferView() override;

    void draw(IScreen &) override;

    Buffer &buffer() {
        return *_buffer;
    }

    const Buffer &buffer() const {
        return *_buffer;
    }

    size_t numberWidth() const {
        return _numberWidth;
    }

    void showLines(bool value) {
        _showLines = value;
        _numberWidth = value * 3;
    }

    Position cursorPosition(Cursor cursor) const;

private:
    std::unique_ptr<Buffer> _buffer;

    bool _showLines = true;
    size_t _numberWidth = 3;
};