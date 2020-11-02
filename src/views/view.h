#pragma once

#include <views/iview.h>

class View : public IView {
public:
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

private:
    size_t _x = 0;
    size_t _y = 0;
    size_t _width = 0;
    size_t _height = 0;
};
