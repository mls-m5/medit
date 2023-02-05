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
    static void highlightStatic(Buffer &buffer);
    void highlight(std::shared_ptr<IScope>) override;
    //    void update(const IPalette &palette) override;

    //! @see IHighlight
    int priority() override {
        return -1;
    }

    static void registerPlugin();

private:
};
