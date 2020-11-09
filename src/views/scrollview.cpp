#include "scrollview.h"
#include "text/cursor.h"

void ScrollView::fitPosition(Position localPosition) {
    if (localPosition.y() < yScroll()) {
        yScroll(localPosition.y());
    }
    else if (localPosition.y() + 1 >= height() + yScroll()) {
        yScroll(localPosition.y() - height() + 1);
    }
}
