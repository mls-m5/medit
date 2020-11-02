#pragma once

#include "files/ifile.h"
#include "meditfwd.h"
#include "modes/imode.h"
#include "text/cursor.h"
#include "views/bufferview.h"
#include <memory>

class Editor : public BufferView {
private:
    Cursor _cursor;
    std::unique_ptr<IMode> _mode = nullptr;
    std::unique_ptr<IFile> _file;

public:
    Editor(std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>())
        : BufferView(std::move(buffer)), _cursor(this->buffer()) {}
    Editor(const Editor &) = delete;
    Editor(Editor &&) = default;
    Editor &operator=(const Editor &) = delete;
    Editor &operator=(Editor &&) = default;

    void setFile(std::unique_ptr<IFile> file) {
        _file = std::move(file);
    }

    void save() {
        if (_file) {
            _file->save(buffer());
        }
    }

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

    void keyPress(IEnvironment &env);

    void updateCursor(IScreen &screen) const;
};
