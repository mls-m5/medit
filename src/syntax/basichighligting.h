#pragma once

#include "syntax/ihighlight.h"
#include "syntax/ipalette.h"
#include "text/buffer.h"
#include <memory>

class BasicHighlighting : public IHighlight {
public:
    struct Format;

    BasicHighlighting();
    ~BasicHighlighting();

    //! @see IHighlight
    bool shouldEnable(filesystem::path) override;
    void highlight(std::shared_ptr<IEnvironment>) override;
    void update(const IPalette &palette) override;

    //! @see IHighlight
    int priority() override {
        return -1;
    }

private:
};
