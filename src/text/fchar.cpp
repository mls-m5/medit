#include "fchar.h"
#include "core/archive.h"
#include "text/utf8char.h"

void FChar::visit(Archive &arch) {
    size_t size = 5;
    if (!arch.beginList("c", size)) {
        return;
    }

    arch("", c.at(0));
    arch("", c.at(1));
    arch("", c.at(2));
    arch("", c.at(3));
    arch("", f);

    arch.endChild();

    //    c.visit(arch);
    //    arch("f", f);

    //    arch.beginChild();
    // arch("");
}
