#pragma once

#include "syntax/ihighlight.h"
#include "text/buffer.h"
#include <memory>

class BasicHighlighting : public IHighlight {
public:
    struct Format;

    BasicHighlighting();
    ~BasicHighlighting();

    // @see IHighlight
    bool shouldEnable(filesystem::path) override;
    void highlight(Editor &) override;
    void update(const IPalette &palette) override;

private:
    void colorize(Buffer &buffer);

    std::unique_ptr<Format> _format;
};
