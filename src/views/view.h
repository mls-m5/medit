#pragma once

#include <views/iview.h>

class View : public virtual IView {
public:
    View(IView *parent)
        : _parent{parent} {}
    View(IView *parent, size_t width, size_t height)
        : _parent{parent}
        , _width(width)
        , _height(height) {}
    View(const View &) = delete;
    View(View &&) = delete;
    View &operator=(const View &) = delete;
    View &operator=(View &&) = delete;

    size_t x() const override {
        return _x;
    }

    size_t y() const override {
        return _y;
    }

    size_t width() const override {
        return _width;
    }

    void x(size_t value) override {
        _x = value;
    }

    void y(size_t value) override {
        _y = value;
    }

    size_t height() const override {
        return _height;
    }

    void width(size_t value) override {
        _width = value;
    }

    void height(size_t value) override {
        _height = value;
    }

    bool visible() const {
        return _visible;
    }

    void visible(bool value) {
        _visible = value;
    }

    bool isInside(int x, int y) override {
        return x >= static_cast<int>(_x) && y >= static_cast<int>(_y) &&
               x <= static_cast<int>(_x + _width) &&
               y < static_cast<int>(_y + _height);
    }

    IView *parent() override {
        return _parent;
    }

    IWindow *window() override {
        if (_parent) {
            return _parent->window();
        }
        return nullptr;
    }

private:
    IView *_parent = nullptr;
    size_t _x = 0;
    size_t _y = 0;
    size_t _width = 10;
    size_t _height = 1;
    bool _visible = true;
};
