#pragma once

#include "filesystem.h"
#include "meditfwd.h"
#include "text/rawbuffer.h"
#include <string_view>

class IFile {
public:
    virtual void load(RawBuffer &) = 0;
    virtual void save(const RawBuffer &) = 0;
    virtual std::string representation() const = 0;
    virtual filesystem::path path() const = 0;

    virtual ~IFile() = default;
};
