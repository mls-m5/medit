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

    void fitPosition(Position localPosition);

    void contentHeight(size_t value);

private:
    size_t _yScroll = 0;
    size_t _contentHeight = 0;
};
