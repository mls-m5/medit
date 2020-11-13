#pragma once

#include "syntax/ihighlight.h"
#include "text/buffer.h"

class BasicHighlighting : public IHighlight {
public:
    // @see IHighlight
    bool shouldEnable(filesystem::path) override;
    void highlight(Editor &) override;
};
