#pragma once

#include "meditfwd.h"
#include "text/formattype.h"
#include "text/fstringview.h"
#include "views/scrollview.h"
#include <memory>
#include <vector>

class BufferView : public ScrollView {
public:
    BufferView(IView *parent, std::shared_ptr<Buffer> buffer);
    ~BufferView() override;

    BufferView(const BufferView &) = delete;
    BufferView(BufferView &&) = delete;
    BufferView &operator=(const BufferView &) = delete;
    BufferView &operator=(BufferView &&) = delete;

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

    void bufferChangedEvent();

    void rewrapLines();

    void onResize(size_t oldWidth, size_t oldHeight) override;

    std::shared_ptr<Buffer> _buffer;
    std::vector<FStringView> _virtualLines;

    bool _showLines = false;
    size_t _numberWidth = 3;
    bool _shouldWrap = true;
};
