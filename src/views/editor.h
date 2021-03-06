#pragma once

#include "files/filesystem.h"
#include "keys/ikeysink.h"
#include "meditfwd.h"
#include "text/cursor.h"
#include "text/cursorrange.h"
#include "text/formattype.h"
#include "text/history.h"
#include "views/bufferview.h"
#include <memory>
#include <optional>

class Editor : public View, public IKeySink {
private:
    BufferView _bufferView;
    History _history;
    Cursor _cursor;
    std::optional<Cursor> _selectionAnchor;
    std::shared_ptr<IMode> _mode;
    std::unique_ptr<IFile> _file;
    FormatType _background = 1;

public:
    Editor(std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>());
    ~Editor() override;

    Editor(const Editor &) = delete;
    Editor(Editor &&) = delete;
    Editor &operator=(const Editor &) = delete;
    Editor &operator=(Editor &&) = delete;

    void file(std::unique_ptr<IFile> file);

    IFile *file() {
        return _file.get();
    }

    filesystem::path path();

    void background(FormatType c) {
        _background = c;
    }

    void save();

    void load();

    void undo();
    void redo();

    Buffer &buffer() {
        return _bufferView.buffer();
    }

    void buffer(std::unique_ptr<Buffer> buffer) {
        _bufferView.buffer(std::move(buffer));
    }

    BufferView &bufferView() {
        return _bufferView;
    }

    Cursor cursor() const {
        return _cursor;
    }

    //! @param deselect = true if you want to disable the selectionAnchor
    Cursor cursor(Cursor c, bool deselect = false);
    Cursor cursor(Position position) {
        return cursor({buffer(), position});
    }

    void anchor(Cursor cursor) {
        _selectionAnchor = {cursor};
    }

    void clearSelection() {
        _selectionAnchor = {};
    }

    CursorRange selection() {
        if (_selectionAnchor) {
            return {*_selectionAnchor, _cursor};
        }
        else {
            return {_cursor};
        }
    }

    void selection(CursorRange range) {
        anchor(range.begin());
        cursor(range.end());
    }

    void mode(std::shared_ptr<IMode> mode);

    IMode &mode() {
        return *_mode;
    }

    void showLines(bool value) {
        _bufferView.showLines(value);
    }

    bool keyPress(std::shared_ptr<IEnvironment>) override;

    void updateCursor(IScreen &) const override;

    //    void updatePalette(const IPalette &palette);

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
