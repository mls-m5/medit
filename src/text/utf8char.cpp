#include "utf8char.h"
#include "core/archive.h"
#include <stdexcept>

void Utf8Char::visit(Archive &arch) {
    size_t size = 4;
    if (!arch.beginList("c", size)) {
        throw std::runtime_error{"could not parse char"};
        return;
    }

    /// Todo string
    arch("", _data.at(0));
    arch("", _data.at(1));
    arch("", _data.at(2));
    arch("", _data.at(3));

    arch.endChild();
}
