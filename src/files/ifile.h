#pragma once

#include "meditfwd.h"

class IFile {
public:
    virtual void load(Buffer &) = 0;
    virtual void save(const Buffer &) = 0;

    virtual ~IFile() = default;
};
