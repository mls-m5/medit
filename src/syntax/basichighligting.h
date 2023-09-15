#pragma once

#include "syntax/ihighlight.h"
#include "text/buffer.h"
#include <memory>

class BasicHighlighting : public IHighlight {
public:
    struct Format;

    BasicHighlighting();
    BasicHighlighting(const BasicHighlighting &) = delete;
    BasicHighlighting(BasicHighlighting &&) = delete;
    BasicHighlighting &operator=(const BasicHighlighting &) = delete;
    BasicHighlighting &operator=(BasicHighlighting &&) = delete;

    ~BasicHighlighting() override;

    //! @see IHighlight
    bool shouldEnable(std::filesystem::path) override;
    static void highlightStatic(Buffer &buffer);
    void highlight(Buffer &buffer) override;

    //! @see IHighlight
    int priority() override {
        return -1;
    }

private:
};
