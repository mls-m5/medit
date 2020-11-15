#pragma once

#include "keys/bufferkeymap.h"
#include "keys/keymap.h"
#include "modes/imode.h"
#include <memory>
#include <string>

class Mode : public IMode {

    std::string _name;
    KeyMap _keyMap;
    std::string _commandBuffer;
    //    bool _bufferMode = false;
    //    std::vector<KeyEvent> _bufferTriggers;
    FString _buffer;
    BufferKeyMap _bufferMap;
    std::unique_ptr<IMode> _parent;
    std::function<void(Editor &)> _start;
    std::function<void(Editor &)> _exit;

public:
    Mode(std::string name,
         KeyMap map,
         std::unique_ptr<IMode> parent = {},
         BufferKeyMap bufferMap = {});

    bool keyPress(IEnvironment &env) override;

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

    std::string_view name() override {
        return _name;
    }
};
