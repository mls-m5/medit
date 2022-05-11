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

    IFile *file();

    filesystem::path path();

    void background(FormatType c);

    void save();

    void load();

    void undo();
    void redo();

    Buffer &buffer();

    void buffer(std::unique_ptr<Buffer> buffer);

    BufferView &bufferView();

    Cursor cursor() const;

    //! @param deselect = true if you want to disable the selectionAnchor
    Cursor cursor(Cursor c, bool deselect = false);
    Cursor cursor(Position position);

    void anchor(Cursor cursor);

    void clearSelection();

    CursorRange selection();

    void selection(CursorRange range);

    void mode(std::shared_ptr<IMode> mode);

    IMode &mode();

    void showLines(bool value);

    bool keyPress(std::shared_ptr<IEnvironment>) override;

    void updateCursor(IScreen &) const override;

    void draw(IScreen &) override;

    void height(size_t value) override;

    void width(size_t value) override;

    size_t width() const override;

    size_t height() const override;

    void x(size_t x) override;

    void y(size_t y) override;

    size_t x() const override;

    size_t y() const override;

    //! Make sure that the buffer with the cursor is scrolled so that the cursor
    //! is visible
    void fitCursor();
};
