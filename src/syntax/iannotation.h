#pragma once

#include "files/filesystem.h"
#include "meditfwd.h"

class IAnnotation {
public:
    virtual ~IAnnotation() = default;

    //! Returns 0 if there is a error or true else
    virtual bool annotate(IEnvironment &env) = 0;

    virtual bool shouldEnable(filesystem::path) = 0;
};
