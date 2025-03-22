#pragma once

#include "meditfwd.h"
#include <filesystem>

class IAnnotation {
public:
    IAnnotation() = default;
    IAnnotation(const IAnnotation &) = delete;
    IAnnotation(IAnnotation &&) = delete;
    IAnnotation &operator=(const IAnnotation &) = delete;
    IAnnotation &operator=(IAnnotation &&) = delete;
    virtual ~IAnnotation() = default;

    //! Returns 0 if there is a error or true else
    virtual bool annotate(Buffer &buffer) = 0;

    virtual bool shouldEnable(std::filesystem::path) = 0;
};
