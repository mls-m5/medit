#pragma once

#include "meditfwd.h"
#include "navigation/inavigation.h"
#include <memory>

class HeaderNavigation : public INavigation {
public:
    bool gotoSymbol(std::shared_ptr<IEnvironment> env) override;

    static void registerPlugin();
};
