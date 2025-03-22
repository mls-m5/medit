#pragma once

#include "meditfwd.h"

class IHighlight {
public:
    IHighlight() = default;
    IHighlight(const IHighlight &) = delete;
    IHighlight(IHighlight &&) = delete;
    IHighlight &operator=(const IHighlight &) = delete;
    IHighlight &operator=(IHighlight &&) = delete;
    virtual ~IHighlight() = default;

    //! If this specific highlighting should be used for this file
    //    virtual bool shouldEnable(std::filesystem::path) = 0;

    /// @return true if highlighting was supported and applied
    virtual bool highlight(Buffer &) = 0;

    //! High priority means that it should be selected first
    virtual int priority() = 0;
};
