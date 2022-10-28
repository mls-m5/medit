#pragma once

#include "meditfwd.h"
#include "navigation/inavigation.h"
#include <memory>

class ClangNavigation : public INavigation {
public:
    bool gotoSymbol(std::shared_ptr<IScope> env) override;

    static void registerPlugin();
};
