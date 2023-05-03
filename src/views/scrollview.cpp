#include "scrollview.h"
#include "text/position.h"

void ScrollView::fitPosition(Position localPosition) {
    if (localPosition.x() < xScroll()) {
        xScroll(localPosition.x());
    }
    else if (localPosition.x() + 1 >= height() + xScroll()) {
        xScroll(localPosition.x() - height() + 1);
    }

    if (localPosition.y() < yScroll()) {
        yScroll(localPosition.y());
    }
    else if (localPosition.y() + 1 >= height() + yScroll()) {
        yScroll(localPosition.y() - height() + 1);
    }
}
