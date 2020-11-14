#pragma once

#include "files/filesystem.h"
#include "meditfwd.h"

class IHighlight {
public:
    virtual ~IHighlight() = default;

    //! If this specific highlighting should be used for this file
    virtual bool shouldEnable(filesystem::path) = 0;
    virtual void highlight(Editor &) = 0;
    virtual void update(const Palette &palette) = 0;
};
