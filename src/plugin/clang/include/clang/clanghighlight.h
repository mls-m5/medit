#pragma once

#include "meditfwd.h"
#include "syntax/ihighlight.h"
#include "syntax/palette.h"

class ClangHighlight : public IHighlight {
public:
    //! @see IHighlight
    bool shouldEnable(filesystem::path path) override;
    void highlight(std::shared_ptr<IScope> env) override;

    //! @see IHighlight
    void update(const Palette &palette) override;

    //! @see IHighlight
    int priority() override {
        return 100;
    }
};
