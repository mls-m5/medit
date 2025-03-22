#pragma once

#include "meditfwd.h"

class IFormat {
public:
    IFormat() = default;
    IFormat(const IFormat &) = delete;
    IFormat(IFormat &&) = delete;
    IFormat &operator=(const IFormat &) = delete;
    IFormat &operator=(IFormat &&) = delete;

    virtual ~IFormat() = default;

    //! @return true if tried to format file, to prevent other low prio
    //! formatting
    virtual bool format(Editor &) = 0;
};
