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

    Position cursorPosition(Cursor cursor) const;

    /// Close the buffer and open another buffer previously opened in editor
    /// @return true if another buffer was found and false if no other buffer
    /// existed
    /// Note that if the bufferview resides in a Editor use the editors
    /// closeBuffer() function instead to avoid messing up the cursor pointer
    bool closeBuffer();

private:
    void subscribeToBuffer();

    void unsubscribe();

    std::shared_ptr<Buffer> _buffer;

    bool _showLines = false;
    size_t _numberWidth = 3;

    std::vector<std::weak_ptr<Buffer>> _openedBuffers;
};
