#pragma once

#include "meditfwd.h"
#include <memory>

class ClangNavigation {
public:
    static void gotoSymbol(std::shared_ptr<IEnvironment> env);
};
