#include "fstringview.h"
#include "buffer.h"
#include "core/archive.h"
#include <cassert>

void save(Archive &arch, FStringView &value) {
    auto str = FString{value};
    str.visit(arch);
}

FStringView::FStringView(const FString &str)
    : _data{str.data()}
    , _size{str.size()} {}

FStringView::FStringView(const Cursor begin, Cursor end) {
    assert(&begin.buffer() == &end.buffer());
    assert(begin.y() == end.y());
    assert(begin.x() <= end.x());
    auto &line = begin.buffer().lines().at(begin.y());
    _data = &line.at(begin.x());
    _size = end.x() - begin.x();
}

FStringView::operator FString() const {
    return FString{begin(), end()};
}

size_t FStringView::find(Utf8Char c, size_t start) const {
    for (size_t i = start; i < size(); ++i) {
        if (at(i) == c) {
            return i;
        }
    }

    return npos;
}
