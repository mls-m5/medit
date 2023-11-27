#pragma once

#include "meditfwd.h"
#include <cstddef>

class IView {
public:
    IView(const IView &) = delete;
    IView(IView &&) = delete;
    IView &operator=(const IView &) = delete;
    IView &operator=(IView &&) = delete;
    IView() = default;

    virtual void draw(IScreen &) = 0;
    virtual void onResize(size_t oldWidth, size_t oldHeight) = 0;

    virtual ~IView() = default;

    virtual size_t x() const = 0;
    virtual size_t y() const = 0;
    virtual size_t width() const = 0;
    virtual size_t height() const = 0;
    virtual void x(size_t value) = 0;
    virtual void y(size_t value) = 0;
    virtual void width(size_t) = 0;
    virtual void height(size_t) = 0;
    virtual bool isInside(int x, int y) = 0;

    virtual IView *parent() = 0;

    virtual IWindow *window() = 0;
};
