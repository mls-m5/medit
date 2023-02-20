#pragma once

#include "keys/ikeysink.h"
#include "meditfwd.h"
// #include "sol/sol.hpp"
#include "text/cursor.h"
#include "text/cursorrange.h"
#include "text/formattype.h"
#include "views/bufferview.h"
#include <filesystem>
#include <memory>
#include <optional>

class Editor : public View, public IKeySink {
public:
    Editor(IView *parent, std::shared_ptr<Buffer> buffer);
    ~Editor() override;

    Editor(const Editor &) = delete;
    Editor(Editor &&) = delete;
    Editor &operator=(const Editor &) = delete;
    Editor &operator=(Editor &&) = delete;

    void file(std::unique_ptr<IFile> file);

    IFile *file();

    std::filesystem::path path();

    void background(FormatType c);

    void save();
    void load();

    void undo();
    void undoMajor();
    void redo();

    Buffer &buffer();

    void buffer(std::shared_ptr<Buffer> buffer);

    BufferView &bufferView();

    Cursor cursor() const;

    //! @param deselect = true if you want to disable the selectionAnchor
    Cursor cursor(Cursor c, bool deselect = false);
    Cursor cursor(Position position);

    void anchor(Cursor cursor);

    void clearSelection();

    CursorRange selection();

    //! Set selection and make selection stay when moving cursor
    //! If you just want to move the cursor, use cursor(...) instead
    void selection(CursorRange range);

    void mode(std::shared_ptr<IMode> mode);

    IMode &mode();

    void showLines(bool value);

    bool keyPress(std::shared_ptr<IScope>) override;

    bool mouseDown(int x, int y) override;

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

private:
    BufferView _bufferView;
    Cursor _cursor;
    std::optional<Cursor> _selectionAnchor;
    std::shared_ptr<IMode> _mode;
    FormatType _background = 1;

    //    sol::table _luaMode;
};
