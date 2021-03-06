#pragma once

#include "keys/bufferkeymap.h"
#include "keys/keymap.h"
#include "modes/imode.h"
#include <memory>
#include <string>

class Mode : public IMode, public std::enable_shared_from_this<Mode> {
    std::string _name;
    KeyMap _keyMap;
    std::string _commandBuffer;
    //    bool _bufferMode = false;
    //    std::vector<KeyEvent> _bufferTriggers;
    FString _buffer;
    BufferKeyMap _bufferMap;
    std::shared_ptr<IMode> _parent;
    std::function<void(Editor &)> _start;
    std::function<void(Editor &)> _exit;
    CursorStyle _cursorStyle = CursorStyle::Block;

public:
    Mode(std::string name,
         KeyMap map,
         CursorStyle cursorStyle = CursorStyle::Block,
         std::shared_ptr<IMode> parent = {},
         BufferKeyMap bufferMap = {});

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
};
