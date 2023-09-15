#pragma once

#include "meditfwd.h"
#include <memory>

class INavigation {
public:
    INavigation(const INavigation &) = delete;
    INavigation(INavigation &&) = delete;
    INavigation &operator=(const INavigation &) = delete;
    INavigation &operator=(INavigation &&) = delete;

    INavigation() = default;
    virtual ~INavigation() = default;

    virtual bool gotoSymbol(std::shared_ptr<IEnvironment> env) = 0;
};
