#pragma once

#include "meditfwd.h"
#include <string_view>

class IFile {
public:
    virtual void load(Buffer &) = 0;
    virtual void save(const Buffer &) = 0;
    virtual std::string representation() const = 0;

    virtual ~IFile() = default;
};
