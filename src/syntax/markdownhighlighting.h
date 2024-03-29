#pragma once

#include "syntax/ihighlight.h"
#include "text/buffer.h"
#include <memory>

class MarkdownHighlighting : public IHighlight {
public:
    struct Format;

    MarkdownHighlighting();

    MarkdownHighlighting(const MarkdownHighlighting &) = delete;
    MarkdownHighlighting(MarkdownHighlighting &&) = delete;
    MarkdownHighlighting &operator=(const MarkdownHighlighting &) = delete;
    MarkdownHighlighting &operator=(MarkdownHighlighting &&) = delete;

    ~MarkdownHighlighting() override;

    //! @see IHighlight
    static bool shouldEnable(std::filesystem::path) ;

    bool highlight(Buffer &buffer) override;

    //! @see IHighlight
    int priority() override {
        return 1;
    }
};
