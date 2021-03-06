#pragma once

#include "meditfwd.h"
#include "navigation/inavigation.h"
#include <memory>

class ClangNavigation : public INavigation {
public:
    void gotoSymbol(std::shared_ptr<IEnvironment> env) override;
};
