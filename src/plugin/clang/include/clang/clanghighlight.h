#pragma once

#include "meditfwd.h"
#include "syntax/ihighlight.h"
#include "syntax/ipalette.h"

class ClangHighlight : public IHighlight {
public:
    //! @see IHighlight
    bool shouldEnable(filesystem::path path) override;
    void highlight(std::shared_ptr<IEnvironment> env) override;

    //! @see IHighlight
    void update(const IPalette &palette) override;

    //! @see IHighlight
    int priority() override {
        return 100;
    }
};
