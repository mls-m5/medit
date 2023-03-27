#pragma once

#include "files/filesystem.h"
#include "meditfwd.h"

class IHighlight {
public:
    virtual ~IHighlight() = default;

    //! If this specific highlighting should be used for this file
    virtual bool shouldEnable(filesystem::path) = 0;
    virtual void highlight(std::shared_ptr<IEnvironment> env) = 0;
    //    virtual void update(const IPalette &palette) = 0;

    //! High priority means that it should be selected first
    virtual int priority() = 0;
};
