#include "scrollview.h"
#include "text/position.h"

void ScrollView::fitPosition(Position localPosition) {
    if (localPosition.x() < xScroll() + _xScrollMargin) {
        xScroll(localPosition.x());
    }
    else if (localPosition.x() + 1 + _xScrollMargin >= height() + xScroll()) {
        xScroll(localPosition.x() - height() + 1);
    }

    if (localPosition.y() < yScroll() + _yScrollMargin) {
        if (localPosition.y() < _yScrollMargin) {
            yScroll(0);
        }
        else {
            yScroll(localPosition.y() - _yScrollMargin);
        }
    }
    else if (localPosition.y() + 1 + _yScrollMargin >= height() + yScroll()) {
        if (localPosition.y() + 1 + _yScrollMargin >= _contentHeight) {
            if (height() > _contentHeight) {
                yScroll(0);
            }
            else {
                yScroll(_contentHeight - height());
            }
        }
        else {
            yScroll(localPosition.y() - height() + 1 + _yScrollMargin);
        }
    }
}
