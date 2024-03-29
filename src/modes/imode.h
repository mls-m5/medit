#pragma once

#include "meditfwd.h"
#include "screen/cursorstyle.h"
#include <functional>
#include <memory>
#include <string_view>

class IMode {
public:
    IMode(const IMode &) = delete;
    IMode(IMode &&) = delete;
    IMode &operator=(const IMode &) = delete;
    IMode &operator=(IMode &&) = delete;
    IMode() = default;
    virtual ~IMode() = default;

    /// @return true if handled by this mode
    virtual bool keyPress(std::shared_ptr<IEnvironment>) = 0;
    virtual std::string_view name() const = 0;

    virtual void start(Editor &) = 0;
    virtual void exit(Editor &) = 0;
    virtual CursorStyle cursorStyle() const = 0;

    /// If only whole lines can be selected
    virtual bool isBlockSelection() const = 0;

    virtual struct FStringView buffer() const = 0;
    virtual int repetitions() const = 0;

    /// For example for normal mode you would not want to select past the last
    /// character
    virtual bool shouldSelectPlusOne() const = 0;

    /// True for all modes but for vim-insert mode. If true. Every key press
    /// triggers a major change mark in the buffers history
    virtual bool isEverythingMajor() const = 0;
};
