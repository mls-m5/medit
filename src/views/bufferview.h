#pragma once

#include "meditfwd.h"
#include "text/formattype.h"
#include "views/scrollview.h"
#include <memory>
#include <vector>

class BufferView : public ScrollView {
public:
    BufferView(IView *parent, std::shared_ptr<Buffer> buffer);
    ~BufferView() override;

    void draw(IScreen &) override;

    void drawSpecial(IScreen &, const CursorRange &range, FormatType f) const;

    Buffer &buffer();

    const Buffer &buffer() const;

    void buffer(std::shared_ptr<Buffer> buffer);

    size_t numberWidth() const;

    void showLines(bool value);

    Position cursorPosition(Position cursor) const;

private:
    void subscribeToBuffer();

    void unsubscribe();

    std::shared_ptr<Buffer> _buffer;

    bool _showLines = false;
    size_t _numberWidth = 3;
};
