#pragma once

#include "views/view.h"

class ScrollView : public View {
public:
    using View::View;

    void yScroll(size_t value) {
        _yScroll = value;
    }

    size_t yScroll() const {
        return _yScroll;
    }

    void xScroll(size_t value) {
        _xScroll = value;
    }

    size_t xScroll() const {
        return _xScroll;
    }

    void fitPosition(Position localPosition);

private:
    size_t _yScroll = 0;
    size_t _contentHeight = 0;

    size_t _xScroll = 0;
    size_t _contentWidth = 0;
};
