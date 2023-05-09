#pragma once

#include "meditfwd.h"
#include <filesystem>
#include <memory>

class IAnnotation {
public:
    virtual ~IAnnotation() = default;

    //! Returns 0 if there is a error or true else
    virtual bool annotate(Buffer &buffer) = 0;

    virtual bool shouldEnable(std::filesystem::path) = 0;
};
