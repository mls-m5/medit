#pragma once

#include "meditfwd.h"

class IFormat {
public:
    virtual ~IFormat() = default;

    //! @return true if tried to format file, to prevent other low prio
    //! formatting
    virtual bool format(Editor &) = 0;
};
