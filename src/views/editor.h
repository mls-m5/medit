#pragma once

#include "files/filesystem.h"
#include "keys/ikeysink.h"
#include "meditfwd.h"
#include "text/cursor.h"
#include "text/formattype.h"
#include "views/bufferview.h"
#include <memory>

class Editor : public View, public IKeySink {
private:
    BufferView _bufferView;
    Cursor _cursor;
    std::unique_ptr<IMode> _mode;
    std::unique_ptr<IFile> _file;
    FormatType _background;

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

    Cursor cursor() const {
        return _cursor;
    }

    Cursor cursor(Cursor c);

    void mode(std::unique_ptr<IMode> mode);

    void showLines(bool value) {
        _bufferView.showLines(value);
    }

    bool keyPress(IEnvironment &) override;

    void updateCursor(IScreen &) const override;

    void updatePalette(const IPalette &palette);

    void draw(IScreen &) override;

    void height(size_t value) override {
        View::height(value);
        _bufferView.height(value);
    }

    void width(size_t value) override {
        View::width(value);
        _bufferView.width(value);
    }

    size_t width() const override {
        return View::width();
    }

    size_t height() const override {
        return View::height();
    }

    void x(size_t x) override {
        View::x(x);
        _bufferView.x(x);
    }

    void y(size_t y) override {
        View::y(y);
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
