#pragma once

class IScrollView {
    IScrollView() = default;
    IScrollView(const IScrollView &) = default;
    IScrollView(IScrollView &&) = default;
    IScrollView &operator=(const IScrollView &) = default;
    IScrollView &operator=(IScrollView &&) = default;

    virtual ~IScrollView() = default;
};
