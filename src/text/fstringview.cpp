#include "fstringview.h"
#include "core/archive.h"

void save(Archive &arch, FStringView &value) {
    auto str = FString{value};
    str.visit(arch);
}
