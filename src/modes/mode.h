#pragma once

#include "keys/bufferkeymap.h"
#include "keys/keymap.h"
#include "meditfwd.h"
#include "modes/imode.h"
#include "screen/cursorstyle.h"
#include <memory>
#include <string>

class Mode : public IMode, public std::enable_shared_from_this<Mode> {
    std::string _name;
    KeyMap _keyMap;
    std::string _commandBuffer;
    FString _buffer;
    BufferKeyMap _bufferMap;
    std::shared_ptr<IMode> _parent;
    std::function<void(Editor &)> _start;
    std::function<void(Editor &)> _exit;
    CursorStyle _cursorStyle = CursorStyle::Block;
    bool _isBlockSelection = false;
    bool _shouldEnableNumbers = false;
    int _repetitions = 0;

public:
    Mode(std::string name, KeyMap map, std::shared_ptr<IMode> parent = {});

    bool keyPress(std::shared_ptr<IEnvironment>) override;

    void startCallback(std::function<void(Editor &)> f) {
        _start = f;
    }
    void exitCallback(std::function<void(Editor &)> f) {
        _exit = f;
    }

    void start(Editor &editor) override {
        if (_start) {
            _start(editor);
        }
    }
    void exit(Editor &editor) override {
        if (_exit) {
            _exit(editor);
        }
    }

    std::string_view name() const override {
        return _name;
    }

    CursorStyle cursorStyle() const override {
        return _cursorStyle;
    }

    bool isBlockSelection() const override {
        return _isBlockSelection;
    }

    int repetitions() const override {
        return std::max(1, _repetitions);
    }

    Mode &cursorStyle(CursorStyle style) {
        _cursorStyle = style;
        return *this;
    }

    Mode &bufferMap(BufferKeyMap map) {
        _bufferMap = std::move(map);
        return *this;
    }

    Mode &isBlockSelection(bool value) {
        _isBlockSelection = value;
        return *this;
    }

    Mode &shouldEnableNumbers(bool value) {
        _shouldEnableNumbers = value;
        return *this;
    }

    const FString &buffer() const override {
        return _buffer;
    }
};
