#pragma once

#include "meditfwd.h"
#include "text/formattype.h"
#include "views/scrollview.h"
#include <memory>

class BufferView : public ScrollView {
public:
    BufferView(std::shared_ptr<Buffer> buffer);
    ~BufferView() override;

    void draw(IScreen &) override;

    void drawSpecial(IScreen &, CursorRange &range, FormatType f);

    Buffer &buffer();

    const Buffer &buffer() const;

    void buffer(std::shared_ptr<Buffer> buffer);

    size_t numberWidth() const;

    void showLines(bool value);

    Position cursorPosition(Cursor cursor) const;

private:
    std::shared_ptr<Buffer> _buffer;

    bool _showLines = false;
    size_t _numberWidth = 3;
};
