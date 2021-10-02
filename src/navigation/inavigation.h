#pragma once

#include "meditfwd.h"
#include <memory>

class INavigation {
public:
    virtual bool gotoSymbol(std::shared_ptr<IEnvironment> env) = 0;

    virtual ~INavigation() = default;
};
