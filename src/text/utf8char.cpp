#include "utf8char.h"
#include "core/archive.h"

void Utf8Char::visit(Archive &arch) {
    size_t size = 4;
    if (!arch.beginList("c", size)) {
        return;
    }

    /// Todo string
    arch("", _data.at(0));
    arch("", _data.at(1));
    arch("", _data.at(2));
    arch("", _data.at(3));

    arch.endChild();
}
