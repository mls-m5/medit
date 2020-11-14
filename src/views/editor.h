#pragma once

#include "files/filesystem.h"
#include "keys/ikeysink.h"
#include "meditfwd.h"
#include "modes/imode.h"
#include "text/cursor.h"
#include "views/bufferview.h"
#include <memory>

class Editor : public View, public IKeySink {
private:
    BufferView _bufferView;
    Cursor _cursor;
    std::unique_ptr<IMode> _mode;
    std::unique_ptr<IFile> _file;
    //    IHighlight *_highlight = nullptr;

public:
    Editor(std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>());
    ~Editor() override;

    void file(std::unique_ptr<IFile> file);

    IFile *file() {
        return _file.get();
    }

    filesystem::path path();

    void save();

    void load();

    auto &buffer() {
        return _bufferView.buffer();
    }

    auto &bufferView() {
        return _bufferView;
    }

    auto cursor() const {
        return _cursor;
    }

    auto cursor(Cursor c) {
        _cursor = c;
        const auto &lines = _bufferView.buffer().lines();
        if (_cursor.y() > lines.size()) {
            _cursor.y(lines.size());
        }
        fitCursor();
    }

    void mode(std::unique_ptr<IMode> mode) {
        _mode = move(mode);
    }

    void showLines(bool value) {
        _bufferView.showLines(value);
    }

    bool keyPress(IEnvironment &) override;

    void updateCursor(IScreen &) const override;

    void draw(IScreen &) override;

    void height(size_t value) override {
        View::height(value);
        _bufferView.height(value);
    }

    void width(size_t value) override {
        View::width(value);
        _bufferView.width(value);
    }

    void x(size_t x) override {
        View::x();
        _bufferView.x(x);
    }

    void y(size_t y) override {
        View::y();
        _bufferView.y(y);
    }

    size_t x() const override {
        return View::x();
    }

    size_t y() const override {
        return View::y();
    }

    //! Make sure that the buffer with the cursor is scrolled so that the cursor
    //! is visible
    void fitCursor();
};
