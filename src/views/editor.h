#pragma once

//#include "text/buffer.h"
#include "meditfwd.h"
#include "modes/imode.h"
#include "text/cursor.h"
#include "views/bufferview.h"

class Editor : public BufferView {
private:
    Cursor _cursor;
    std::unique_ptr<IMode> _mode = nullptr;

public:
    Editor(std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>())
        : BufferView(std::move(buffer)) {}
    Editor(const Editor &) = delete;
    Editor(Editor &&) = default;
    Editor &operator=(const Editor &) = delete;
    Editor &operator=(Editor &&) = default;

    auto &cursor() {
        return _cursor;
    }

    auto cursor() const {
        return _cursor;
    }

    auto cursor(Cursor c) {
        _cursor = c;
    }

    void mode(std::unique_ptr<IMode> mode) {
        _mode = move(mode);
    }

    void keyPress(const KeyEvent &key);

    void updateCursor(IScreen &screen) const;
};
